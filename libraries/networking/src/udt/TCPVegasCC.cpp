//
//  TCPVegasCC.cpp
//  libraries/networking/src/udt
//
//  Created by Stephen Birarda on 2016-09-20.
//  Copyright 2016 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "TCPVegasCC.h"

#include <QtCore/QDebug>
#include <QtCore/QtGlobal>

using namespace udt;
using namespace std::chrono;

TCPVegasCC::TCPVegasCC() {
    _packetSendPeriod = 0.0;
    _congestionWindowSize = 2;

    setAckInterval(1); // TCP sends an ACK for every packet received

    // set our minimum RTT variables to the maximum possible value
    // we can't do this as a member initializer until our VS has support for constexpr
    _currentMinRTT = std::numeric_limits<int>::max();
    _baseRTT = std::numeric_limits<int>::max();
}

bool TCPVegasCC::onACK(SequenceNumber ack, p_high_resolution_clock::time_point receiveTime) {
    auto it = _sentPacketTimes.find(ack);

    auto previousAck = _lastACK;
    _lastACK = ack;

    if (it != _sentPacketTimes.end()) {

        // calculate the RTT (receive time - time ACK sent)
        int lastRTT = duration_cast<microseconds>(receiveTime - it->second).count();

        const int MAX_RTT_SAMPLE_MICROSECONDS = 10000000;

        if (lastRTT < 0) {
            Q_ASSERT_X(false, __FUNCTION__, "calculated an RTT that is not > 0");
            return false;
        } else if (lastRTT == 0) {
            // we do not allow a zero microsecond RTT (as per the UNIX kernel implementation of TCP Vegas)
            lastRTT = 1;
        } else if (lastRTT > MAX_RTT_SAMPLE_MICROSECONDS) {
            // we cap the lastRTT to MAX_RTT_SAMPLE_MICROSECONDS to avoid overflows in window size calculations
            lastRTT = MAX_RTT_SAMPLE_MICROSECONDS;
        }

        if (_ewmaRTT == -1) {
            // first RTT sample - set _ewmaRTT to the value and set the variance to half the value
            _ewmaRTT = lastRTT;
            _rttVariance = lastRTT / 2;
        } else {
            // This updates the RTT using exponential weighted moving average
            // This is the Jacobson's forumla for RTT estimation
            // http://www.mathcs.emory.edu/~cheung/Courses/455/Syllabus/7-transport/Jacobson-88.pdf

            // Estimated RTT = (1 - x)(estimatedRTT) + (x)(sampleRTT)
            // (where x = 0.125 via Jacobson)

            // Deviation  = (1 - x)(deviation) + x |sampleRTT - estimatedRTT|
            // (where x = 0.25 via Jacobson)

            static const int RTT_ESTIMATION_ALPHA = 8;
            static const int RTT_ESTIMATION_VARIANCE_ALPHA = 4;

            _ewmaRTT = (_ewmaRTT * (RTT_ESTIMATION_ALPHA - 1) + lastRTT) / RTT_ESTIMATION_ALPHA;
            _rttVariance = (_rttVariance * (RTT_ESTIMATION_VARIANCE_ALPHA- 1)
                            + abs(lastRTT - _ewmaRTT)) / RTT_ESTIMATION_VARIANCE_ALPHA;
        }

        // add 1 to the number of ACKs during this RTT
        ++_numACKs;

        // keep track of the lowest RTT during connection
        _baseRTT = std::min(_baseRTT, lastRTT);

        // find the min RTT during the last RTT
        _currentMinRTT = std::min(_currentMinRTT, lastRTT);

        auto sinceLastAdjustment = duration_cast<microseconds>(p_high_resolution_clock::now() - _lastAdjustmentTime).count();
        if (sinceLastAdjustment >= _ewmaRTT) {
            performCongestionAvoidance(ack);
        }

        // remove this sent packet time from the hash
        _sentPacketTimes.erase(it);
    }

    ++_numACKSinceFastRetransmit;

    // perform the fast re-transmit check if this is a duplicate ACK or if this is the first or second ACK
    // after a previous fast re-transmit
    if (ack == previousAck || _numACKSinceFastRetransmit < 3) {
        // we may need to re-send ackNum + 1 if it has been more than our estimated timeout since it was sent

        auto it = _sentPacketTimes.find(ack + 1);
        if (it != _sentPacketTimes.end()) {
            auto estimatedTimeout = _ewmaRTT + _rttVariance * 4;

            auto now = p_high_resolution_clock::now();
            auto sinceSend = duration_cast<microseconds>(now - it->second).count();

            if (sinceSend >= estimatedTimeout) {
                // break out of slow start, we've decided this is loss
                _slowStart = false;

                // reset the fast re-transmit counter
                _numACKSinceFastRetransmit = 0;

                // return true so the caller knows we needed a fast re-transmit
                return true;
            }
        }

        // if this is the 3rd duplicate ACK, we fallback to Reno's fast re-transmit
        static const int RENO_FAST_RETRANSMIT_DUPLICATE_COUNT = 3;

        ++_duplicateACKCount;

        if (ack == previousAck &&  _duplicateACKCount == RENO_FAST_RETRANSMIT_DUPLICATE_COUNT) {
            // break out of slow start, we just hit loss
            _slowStart = false;

            // reset our fast re-transmit counters
            _numACKSinceFastRetransmit = 0;
            _duplicateACKCount = 0;

            // return true so the caller knows we needed a fast re-transmit
            return true;
        }
    } else {
        _duplicateACKCount = 0;
    }

    // ACK processed, no fast re-transmit required
    return false;
}

void TCPVegasCC::performCongestionAvoidance(udt::SequenceNumber ack) {
    static int VEGAS_ALPHA_SEGMENTS = 4;
    static int VEGAS_BETA_SEGMENTS = 6;
    static int VEGAS_GAMMA_SEGMENTS = 1;

    // http://pages.cs.wisc.edu/~akella/CS740/S08/740-Papers/BOP94.pdf
    // Use the Vegas algorithm to see if we should
    // increase or decrease the congestion window size, and by how much

    // Grab the minimum RTT seen during the last RTT (since the last performed congestion avoidance)
    
    // Taking the min avoids the effects of delayed ACKs
    // (though congestion may be noticed a bit later)
    int rtt = _currentMinRTT;

    int64_t windowSizeDiff = (int64_t) _congestionWindowSize * (rtt - _baseRTT) / _baseRTT;

    if (_numACKs <= 2) {
        performRenoCongestionAvoidance(ack);
    } else {
        if (_slowStart) {
            if (windowSizeDiff > VEGAS_GAMMA_SEGMENTS) {
                // we're going too fast - this breaks us out of slow start and we switch to linear increase/decrease
                _slowStart = false;

                int expectedWindowSize = _congestionWindowSize * _baseRTT / rtt;
                _baseRTT = std::numeric_limits<int>::max();

                // drop the congestion window size to the expected size, if smaller
                _congestionWindowSize = std::min(_congestionWindowSize, expectedWindowSize + 1);

            } else if (++_slowStartOddAdjust & 1) {
                // we're in slow start and not going too fast
                // this means that once every second RTT we perform exponential congestion window growth
                _congestionWindowSize *= 2;
            }
        } else {
            // this is the normal linear increase/decrease of the Vegas algorithm
            // to figure out where the congestion window should be
            if (windowSizeDiff > VEGAS_BETA_SEGMENTS) {
                // the old congestion window was too fast (difference > beta)
                // so reduce it to slow down
                --_congestionWindowSize;

            } else if (windowSizeDiff < VEGAS_ALPHA_SEGMENTS) {
                // there aren't enough packets on the wire, add more to the congestion window
                ++_congestionWindowSize;
            } else {
                // sending rate seems good, no congestion window adjustment
            }
        }
    }

    // we never allow the congestion window to be smaller than two packets
    static int VEGAS_CW_MIN_PACKETS = 2;
    if (_congestionWindowSize < VEGAS_CW_MIN_PACKETS) {
        _congestionWindowSize = VEGAS_CW_MIN_PACKETS;
    } else if (_congestionWindowSize > udt::MAX_PACKETS_IN_FLIGHT) {
        _congestionWindowSize = udt::MAX_PACKETS_IN_FLIGHT;
    }

    // mark this as the last adjustment time
    _lastAdjustmentTime = p_high_resolution_clock::now();

    // reset our state for the next RTT
    _currentMinRTT = std::numeric_limits<int>::max();

    // reset our count of collected RTT samples
    _numACKs = 0;
}

bool TCPVegasCC::isCongestionWindowLimited() {
    if (_slowStart) {
        return true;
    } else {
        return seqlen(_sendCurrSeqNum, _lastACK) < _congestionWindowSize;
    }
}

void TCPVegasCC::performRenoCongestionAvoidance(SequenceNumber ack) {
    if (!isCongestionWindowLimited()) {
        return;
    }

    int numAcked = _numACKs;

    if (_slowStart) {
        // while in slow start we grow the congestion window by the number of ACKed packets
        // allowing it to grow as high as the slow start threshold
        int congestionWindow = _congestionWindowSize + numAcked;

        if (congestionWindow > udt::MAX_PACKETS_IN_FLIGHT) {
            // we're done with slow start, set the congestion window to the slow start threshold
            _congestionWindowSize = udt::MAX_PACKETS_IN_FLIGHT;

            // figure out how many left over ACKs we should apply using the regular reno congestion avoidance
            numAcked = congestionWindow - udt::MAX_PACKETS_IN_FLIGHT;
        } else {
            _congestionWindowSize = congestionWindow;
            numAcked = 0;
        }
    }

    // grab the size of the window prior to reno additive increase
    int preAIWindowSize = _congestionWindowSize;

    if (numAcked > 0) {
        // Once we are out of slow start, we use additive increase to grow the window slowly.
        // We grow the congestion window by a single packet everytime the entire congestion window is sent.

        // If credits accumulated at a higher preAIWindowSize, apply them gently now.
        if (_ackAICount >= preAIWindowSize) {
            _ackAICount = 0;
            ++_congestionWindowSize;
        }

        // increase the window size by (1 / window size) for every ACK received
        _ackAICount += numAcked;
        if (_ackAICount >= preAIWindowSize) {
            // when _ackAICount % preAIWindowSize == 0 then _ackAICount is 0
            // when _ackAICount % preAIWindowSize != 0 then _ackAICount is _ackAICount - (_ackAICount % preAIWindowSize)

            int delta = _ackAICount / preAIWindowSize;

            _ackAICount -= delta * preAIWindowSize;
            _congestionWindowSize += delta;
        }
    }
}

void TCPVegasCC::onPacketSent(int wireSize, SequenceNumber seqNum, p_high_resolution_clock::time_point timePoint) {
    if (_sentPacketTimes.find(seqNum) == _sentPacketTimes.end()) {
        _sentPacketTimes[seqNum] = timePoint;
    }
}

