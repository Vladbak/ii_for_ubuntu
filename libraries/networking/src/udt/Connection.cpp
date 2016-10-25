//
//  Connection.cpp
//  libraries/networking/src/udt
//
//  Created by Clement on 7/27/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "Connection.h"

#include <QtCore/QThread>

#include <NumericalConstants.h>

#include "../HifiSockAddr.h"
#include "../NetworkLogging.h"

#include "CongestionControl.h"
#include "ControlPacket.h"
#include "Packet.h"
#include "PacketList.h"
#include "Socket.h"

using namespace udt;
using namespace std::chrono;

Connection::Connection(Socket* parentSocket, HifiSockAddr destination, std::unique_ptr<CongestionControl> congestionControl) :
    _parentSocket(parentSocket),
    _destination(destination),
    _congestionControl(move(congestionControl))
{
    Q_ASSERT_X(parentSocket, "Connection::Connection", "Must be called with a valid Socket*");
    
    Q_ASSERT_X(_congestionControl, "Connection::Connection", "Must be called with a valid CongestionControl object");
    _congestionControl->init();
    
    // setup default SYN, RTT and RTT Variance based on the SYN interval in CongestionControl object
    _synInterval = _congestionControl->synInterval();
    
    resetRTT();
    
    // set the initial RTT and flow window size on congestion control object
    _congestionControl->setRTT(_rtt);
    _congestionControl->setMaxCongestionWindowSize(_flowWindowSize);

    // Setup packets
    static const int ACK_PACKET_PAYLOAD_BYTES = sizeof(_lastSentACK) + sizeof(_currentACKSubSequenceNumber)
                            + sizeof(_rtt) + sizeof(int32_t) + sizeof(int32_t) + sizeof(int32_t);
    static const int LIGHT_ACK_PACKET_PAYLOAD_BYTES = sizeof(SequenceNumber);
    static const int ACK2_PAYLOAD_BYTES = sizeof(SequenceNumber);
    static const int NAK_PACKET_PAYLOAD_BYTES = 2 * sizeof(SequenceNumber);
    static const int HANDSHAKE_ACK_PAYLOAD_BYTES = sizeof(SequenceNumber);

    _ackPacket = ControlPacket::create(ControlPacket::ACK, ACK_PACKET_PAYLOAD_BYTES);
    _lightACKPacket = ControlPacket::create(ControlPacket::LightACK, LIGHT_ACK_PACKET_PAYLOAD_BYTES);
    _ack2Packet = ControlPacket::create(ControlPacket::ACK2, ACK2_PAYLOAD_BYTES);
    _lossReport = ControlPacket::create(ControlPacket::NAK, NAK_PACKET_PAYLOAD_BYTES);
    _handshakeACK = ControlPacket::create(ControlPacket::HandshakeACK, HANDSHAKE_ACK_PAYLOAD_BYTES);
}

Connection::~Connection() {
    stopSendQueue();

    // Fail any pending received messages
    for (auto& pendingMessage : _pendingReceivedMessages) {
        _parentSocket->messageFailed(this, pendingMessage.first);
    }
}

void Connection::stopSendQueue() {
    if (auto sendQueue = _sendQueue.release()) {
        // grab the send queue thread so we can wait on it
        QThread* sendQueueThread = sendQueue->thread();
        
        // tell the send queue to stop and be deleted
        
        sendQueue->stop();
        sendQueue->deleteLater();
        
        // since we're stopping the send queue we should consider our handshake ACK not receieved
        _hasReceivedHandshakeACK = false;
        
        // wait on the send queue thread so we know the send queue is gone
        sendQueueThread->quit();
        sendQueueThread->wait();
    }
}

void Connection::resetRTT() {
    _rtt = _synInterval * 10;
    _rttVariance = _rtt / 2;
}

void Connection::setMaxBandwidth(int maxBandwidth) {
    _congestionControl->setMaxBandwidth(maxBandwidth);
}

SendQueue& Connection::getSendQueue() {
    if (!_sendQueue) {

        // we may have a sequence number from the previous inactive queue - re-use that so that the
        // receiver is getting the sequence numbers it expects (given that the connection must still be active)

        // Lasily create send queue
        _sendQueue = SendQueue::create(_parentSocket, _destination);
        _lastReceivedACK = _sendQueue->getCurrentSequenceNumber();

#ifdef UDT_CONNECTION_DEBUG
        qCDebug(networking) << "Created SendQueue for connection to" << _destination;
#endif
        
        QObject::connect(_sendQueue.get(), &SendQueue::packetSent, this, &Connection::packetSent);
        QObject::connect(_sendQueue.get(), &SendQueue::packetSent, this, &Connection::recordSentPackets);
        QObject::connect(_sendQueue.get(), &SendQueue::packetRetransmitted, this, &Connection::recordRetransmission);
        QObject::connect(_sendQueue.get(), &SendQueue::queueInactive, this, &Connection::queueInactive);
        QObject::connect(_sendQueue.get(), &SendQueue::timeout, this, &Connection::queueTimeout);
        QObject::connect(_sendQueue.get(), &SendQueue::shortCircuitLoss, this, &Connection::queueShortCircuitLoss);

        
        // set defaults on the send queue from our congestion control object and estimatedTimeout()
        _sendQueue->setPacketSendPeriod(_congestionControl->_packetSendPeriod);
        _sendQueue->setSyncInterval(_synInterval);
        _sendQueue->setEstimatedTimeout(estimatedTimeout());
        _sendQueue->setFlowWindowSize(std::min(_flowWindowSize, (int) _congestionControl->_congestionWindowSize));
        _sendQueue->setProbePacketEnabled(_congestionControl->shouldProbe());

        // give the randomized sequence number to the congestion control object
        _congestionControl->setInitialSendSequenceNumber(_sendQueue->getCurrentSequenceNumber());
    }
    
    return *_sendQueue;
}

void Connection::queueInactive() {
    // tell our current send queue to go down and reset our ptr to it to null
    stopSendQueue();
    
#ifdef UDT_CONNECTION_DEBUG
    qCDebug(networking) << "Connection to" << _destination << "has stopped its SendQueue.";
#endif
    
    if (!_hasReceivedHandshake || !_isReceivingData) {
#ifdef UDT_CONNECTION_DEBUG
        qCDebug(networking) << "Connection SendQueue to" << _destination << "stopped and no data is being received - stopping connection.";
#endif
        
        deactivate();
    }
}

void Connection::queueTimeout() {
    updateCongestionControlAndSendQueue([this] {
        _congestionControl->onTimeout();
    });
}

void Connection::queueShortCircuitLoss(quint32 sequenceNumber) {
    updateCongestionControlAndSendQueue([this, sequenceNumber] {
        _congestionControl->onLoss(SequenceNumber { sequenceNumber }, SequenceNumber { sequenceNumber });
    });
}

void Connection::sendReliablePacket(std::unique_ptr<Packet> packet) {
    Q_ASSERT_X(packet->isReliable(), "Connection::send", "Trying to send an unreliable packet reliably.");
    getSendQueue().queuePacket(std::move(packet));
}

void Connection::sendReliablePacketList(std::unique_ptr<PacketList> packetList) {
    Q_ASSERT_X(packetList->isReliable(), "Connection::send", "Trying to send an unreliable packet reliably.");
    getSendQueue().queuePacketList(std::move(packetList));
}

void Connection::queueReceivedMessagePacket(std::unique_ptr<Packet> packet) {
    Q_ASSERT(packet->isPartOfMessage());

    auto messageNumber = packet->getMessageNumber();
    auto& pendingMessage = _pendingReceivedMessages[messageNumber];

    pendingMessage.enqueuePacket(std::move(packet));

    while (pendingMessage.hasAvailablePackets()) {
        auto packet = pendingMessage.removeNextPacket();
        _parentSocket->messageReceived(std::move(packet));
    }

    if (pendingMessage.isComplete()) {
        _pendingReceivedMessages.erase(messageNumber);
    }
}

void Connection::sync() {
    if (_isReceivingData) {
        
        // check if we should expire the receive portion of this connection
        // this occurs if it has been 16 timeouts since the last data received and at least 5 seconds
        static const int NUM_TIMEOUTS_BEFORE_EXPIRY = 16;
        static const int MIN_SECONDS_BEFORE_EXPIRY = 5;
        
        auto now = p_high_resolution_clock::now();
        
        auto sincePacketReceive = now - _lastReceiveTime;
        
        if (duration_cast<microseconds>(sincePacketReceive).count() >= NUM_TIMEOUTS_BEFORE_EXPIRY * estimatedTimeout()
            && duration_cast<seconds>(sincePacketReceive).count() >= MIN_SECONDS_BEFORE_EXPIRY ) {
            // the receive side of this connection is expired
            _isReceivingData = false;
            
            // if we don't have a send queue that means the whole connection has expired and we can emit our signal
            // otherwise we'll wait for it to also timeout before cleaning up
            if (!_sendQueue) {

#ifdef UDT_CONNECTION_DEBUG
                qCDebug(networking) << "Connection to" << _destination << "no longer receiving any data and there is currently no send queue - stopping connection.";
#endif
                
                deactivate();
                
                return;
            }
        }
        
        // reset the number of light ACKs or non SYN ACKs during this sync interval
        _lightACKsDuringSYN = 1;
        _acksDuringSYN = 1;

        if (_congestionControl->_ackInterval > 1) {
            // we send out a periodic ACK every rate control interval
            sendACK();
        }
        
        if (_lossList.getLength() > 0) {
            // check if we need to re-transmit a loss list
            // we do this if it has been longer than the current nakInterval since we last sent
            auto now = p_high_resolution_clock::now();
            
            if (duration_cast<microseconds>(now - _lastNAKTime).count() >= _nakInterval) {
                // Send a timeout NAK packet
                sendTimeoutNAK();
            }
        }
    } else if (!_sendQueue) {
        // we haven't received a packet and we're not sending
        // this most likely means we were started erroneously
        // check the start time for this connection and auto expire it after 5 seconds of not receiving or sending any data
        static const int CONNECTION_NOT_USED_EXPIRY_SECONDS = 5;
        auto secondsSinceStart = duration_cast<seconds>(p_high_resolution_clock::now() - _connectionStart).count();
        
        if (secondsSinceStart >= CONNECTION_NOT_USED_EXPIRY_SECONDS) {
            // it's been CONNECTION_NOT_USED_EXPIRY_SECONDS and nothing has actually happened with this connection
            // consider it inactive and emit our inactivity signal
            
#ifdef UDT_CONNECTION_DEBUG
            qCDebug(networking) << "Connection to" << _destination << "did not receive or send any data in last"
                << CONNECTION_NOT_USED_EXPIRY_SECONDS << "seconds - stopping connection.";
#endif
            
            deactivate();
            
            return;
        }
    }
}

void Connection::recordSentPackets(int wireSize, int payloadSize,
                                   SequenceNumber seqNum, p_high_resolution_clock::time_point timePoint) {
    _stats.recordSentPackets(payloadSize, wireSize);

    _congestionControl->onPacketSent(wireSize, seqNum, timePoint);
}

void Connection::recordRetransmission(int wireSize, SequenceNumber seqNum, p_high_resolution_clock::time_point timePoint) {
    _stats.record(ConnectionStats::Stats::Retransmission);

    _congestionControl->onPacketSent(wireSize, seqNum, timePoint);
}

void Connection::sendACK(bool wasCausedBySyncTimeout) {
    static p_high_resolution_clock::time_point lastACKSendTime;
    auto currentTime = p_high_resolution_clock::now();
    
    SequenceNumber nextACKNumber = nextACK();
    Q_ASSERT_X(nextACKNumber >= _lastSentACK, "Connection::sendACK", "Sending lower ACK, something is wrong");
    
    if (nextACKNumber == _lastSentACK) {
        // We already sent this ACK, but check if we should re-send it.
        if (nextACKNumber < _lastReceivedAcknowledgedACK) {
            // we already got an ACK2 for this ACK we would be sending, don't bother
            return;
        }
        
        // We will re-send if it has been more than the estimated timeout since the last ACK
        microseconds sinceLastACK = duration_cast<microseconds>(currentTime - lastACKSendTime);
        
        if (sinceLastACK.count() < estimatedTimeout()) {
            return;
        }
    }
    // we have received new packets since the last sent ACK
    
    // update the last sent ACK
    _lastSentACK = nextACKNumber;


    _ackPacket->reset(); // We need to reset it every time.
    
    // pack in the ACK sub-sequence number
    _ackPacket->writePrimitive(++_currentACKSubSequenceNumber);
    
    // pack in the ACK number
    _ackPacket->writePrimitive(nextACKNumber);
    
    // pack in the RTT and variance
    _ackPacket->writePrimitive(_rtt);
    
    // pack the available buffer size, in packets
    // in our implementation we have no hard limit on receive buffer size, send the default value
    _ackPacket->writePrimitive((int32_t) udt::MAX_PACKETS_IN_FLIGHT);

    if (wasCausedBySyncTimeout) {
        // grab the up to date packet receive speed and estimated bandwidth
        int32_t packetReceiveSpeed = _receiveWindow.getPacketReceiveSpeed();
        int32_t estimatedBandwidth = _receiveWindow.getEstimatedBandwidth();

        // update those values in our connection stats
        _stats.recordReceiveRate(packetReceiveSpeed);
        _stats.recordEstimatedBandwidth(estimatedBandwidth);
        
        // pack in the receive speed and estimatedBandwidth
        _ackPacket->writePrimitive(packetReceiveSpeed);
        _ackPacket->writePrimitive(estimatedBandwidth);
    }
    
    // record this as the last ACK send time
    lastACKSendTime = p_high_resolution_clock::now();
    
    // have the socket send off our packet
    _parentSocket->writeBasePacket(*_ackPacket, _destination);
    
    Q_ASSERT_X(_sentACKs.empty() || _sentACKs.back().first + 1 == _currentACKSubSequenceNumber,
               "Connection::sendACK", "Adding an invalid ACK to _sentACKs");
    
    // write this ACK to the map of sent ACKs
    _sentACKs.push_back({ _currentACKSubSequenceNumber, { nextACKNumber, p_high_resolution_clock::now() }});
    
    // reset the number of data packets received since last ACK
    _packetsSinceACK = 0;
    
    _stats.record(ConnectionStats::Stats::SentACK);
}

void Connection::sendLightACK() {
    SequenceNumber nextACKNumber = nextACK();
    
    if (nextACKNumber == _lastReceivedAcknowledgedACK) {
        // we already got an ACK2 for this ACK we would be sending, don't bother
        return;
    }
    
    // reset the lightACKPacket before we go to write the ACK to it
    _lightACKPacket->reset();
    
    // pack in the ACK
    _lightACKPacket->writePrimitive(nextACKNumber);
    
    // have the socket send off our packet immediately
    _parentSocket->writeBasePacket(*_lightACKPacket, _destination);
    
    _stats.record(ConnectionStats::Stats::SentLightACK);
}

void Connection::sendACK2(SequenceNumber currentACKSubSequenceNumber) {
    // reset the ACK2 Packet before writing the sub-sequence number to it
    _ack2Packet->reset();
    
    // write the sub sequence number for this ACK2
    _ack2Packet->writePrimitive(currentACKSubSequenceNumber);
    
    // send the ACK2 packet
    _parentSocket->writeBasePacket(*_ack2Packet, _destination);
    
    // update the last sent ACK2 and the last ACK2 send time
    _lastSentACK2 = currentACKSubSequenceNumber;
    
    _stats.record(ConnectionStats::Stats::SentACK2);
}

void Connection::sendNAK(SequenceNumber sequenceNumberRecieved) {
    _lossReport->reset(); // We need to reset it every time.
    
    // pack in the loss report
    _lossReport->writePrimitive(_lastReceivedSequenceNumber + 1);
    if (_lastReceivedSequenceNumber + 1 != sequenceNumberRecieved - 1) {
        _lossReport->writePrimitive(sequenceNumberRecieved - 1);
    }
    
    // have the parent socket send off our packet immediately
    _parentSocket->writeBasePacket(*_lossReport, _destination);
    
    // record our last NAK time
    _lastNAKTime = p_high_resolution_clock::now();
    
    _stats.record(ConnectionStats::Stats::SentNAK);
}

void Connection::sendTimeoutNAK() {
    if (_lossList.getLength() > 0) {
        
        int timeoutPayloadSize = std::min((int) (_lossList.getLength() * 2 * sizeof(SequenceNumber)),
                                          ControlPacket::maxPayloadSize());
        
        // construct a NAK packet that will hold all of the lost sequence numbers
        auto lossListPacket = ControlPacket::create(ControlPacket::TimeoutNAK, timeoutPayloadSize);
        
        // Pack in the lost sequence numbers
        _lossList.write(*lossListPacket, timeoutPayloadSize / (2 * sizeof(SequenceNumber)));
        
        // have our parent socket send off this control packet
        _parentSocket->writeBasePacket(*lossListPacket, _destination);
        
        // record this as the last NAK time
        _lastNAKTime = p_high_resolution_clock::now();
        
        _stats.record(ConnectionStats::Stats::SentTimeoutNAK);
    }
}

SequenceNumber Connection::nextACK() const {
    if (_lossList.getLength() > 0) {
        return _lossList.getFirstSequenceNumber() - 1;
    } else {
        return _lastReceivedSequenceNumber;
    }
}

void Connection::sendHandshakeRequest() {
    auto handshakeRequestPacket = ControlPacket::create(ControlPacket::HandshakeRequest, 0);
    _parentSocket->writeBasePacket(*handshakeRequestPacket, _destination);

    _didRequestHandshake = true;
}

bool Connection::processReceivedSequenceNumber(SequenceNumber sequenceNumber, int packetSize, int payloadSize) {
    
    if (!_hasReceivedHandshake) {
        // Refuse to process any packets until we've received the handshake
        // Send handshake request to re-request a handshake

#ifdef UDT_CONNECTION_DEBUG
        qCDebug(networking) << "Received packet before receiving handshake, sending HandshakeRequest";
#endif

        sendHandshakeRequest();

        return false;
    }
    
    _isReceivingData = true;
    
    // mark our last receive time as now (to push the potential expiry farther)
    _lastReceiveTime = p_high_resolution_clock::now();
    
    // check if this is a packet pair we should estimate bandwidth from, or just a regular packet
    if (((uint32_t) sequenceNumber & 0xF) == 0) {
        _receiveWindow.onProbePair1Arrival();
    } else if (((uint32_t) sequenceNumber & 0xF) == 1) {
        // only use this packet for bandwidth estimation if we didn't just receive a control packet in its place
        if (!_receivedControlProbeTail) {
            _receiveWindow.onProbePair2Arrival();
        } else {
            // reset our control probe tail marker so the next probe that comes with data can be used
            _receivedControlProbeTail = false;
        }
        
    }
    _receiveWindow.onPacketArrival();
    
    // If this is not the next sequence number, report loss
    if (sequenceNumber > _lastReceivedSequenceNumber + 1) {
        if (_lastReceivedSequenceNumber + 1 == sequenceNumber - 1) {
            _lossList.append(_lastReceivedSequenceNumber + 1);
        } else {
            _lossList.append(_lastReceivedSequenceNumber + 1, sequenceNumber - 1);
        }

        if (_congestionControl->shouldNAK()) {
            // Send a NAK packet
            sendNAK(sequenceNumber);

            // figure out when we should send the next loss report, if we haven't heard anything back
            _nakInterval = estimatedTimeout();

            int receivedPacketsPerSecond = _receiveWindow.getPacketReceiveSpeed();
            if (receivedPacketsPerSecond > 0) {
                // the NAK interval is at least the _minNAKInterval
                // but might be the time required for all lost packets to be retransmitted
                _nakInterval += (int) (_lossList.getLength() * (USECS_PER_SECOND / receivedPacketsPerSecond));
            }

            // the NAK interval is at least the _minNAKInterval but might be the value calculated above, if that is larger
            _nakInterval = std::max(_nakInterval, _minNAKInterval);
        }
    }
    
    bool wasDuplicate = false;
    
    if (sequenceNumber > _lastReceivedSequenceNumber) {
        // Update largest recieved sequence number
        _lastReceivedSequenceNumber = sequenceNumber;
    } else {
        // Otherwise, it could be a resend, try and remove it from the loss list
        wasDuplicate = !_lossList.remove(sequenceNumber);
    }
    
    // increment the counters for data packets received
    ++_packetsSinceACK;
    
    // check if we need to send an ACK, according to CC params
    if (_congestionControl->_ackInterval == 1) {
        // using a congestion control that ACKs every packet (like TCP Vegas)
        sendACK(true);
    } else if (_congestionControl->_ackInterval > 0 && _packetsSinceACK >= _congestionControl->_ackInterval * _acksDuringSYN) {
        _acksDuringSYN++;
        sendACK(false);
    } else if (_congestionControl->_lightACKInterval > 0
               && _packetsSinceACK >= _congestionControl->_lightACKInterval * _lightACKsDuringSYN) {
        sendLightACK();
        ++_lightACKsDuringSYN;
    }
    
    if (wasDuplicate) {
        _stats.record(ConnectionStats::Stats::Duplicate);
    } else {
        _stats.recordReceivedPackets(payloadSize, packetSize);
    }
    
    return !wasDuplicate;
}

void Connection::processControl(ControlPacketPointer controlPacket) {
    
    // Simple dispatch to control packets processing methods based on their type.
    
    // Processing of control packets (other than Handshake / Handshake ACK)
    // is not performed if the handshake has not been completed.
    
    switch (controlPacket->getType()) {
        case ControlPacket::ACK:
            if (_hasReceivedHandshakeACK) {
                processACK(move(controlPacket));
            }
            break;
        case ControlPacket::LightACK:
            if (_hasReceivedHandshakeACK) {
                processLightACK(move(controlPacket));
            }
            break;
        case ControlPacket::ACK2:
            if (_hasReceivedHandshake) {
                processACK2(move(controlPacket));
            }
            break;
        case ControlPacket::NAK:
            if (_hasReceivedHandshakeACK) {
                processNAK(move(controlPacket));
            }
            break;
        case ControlPacket::TimeoutNAK:
            if (_hasReceivedHandshakeACK) {
                processTimeoutNAK(move(controlPacket));
            }
            break;
        case ControlPacket::Handshake:
            processHandshake(move(controlPacket));
            break;
        case ControlPacket::HandshakeACK:
            processHandshakeACK(move(controlPacket));
            break;
        case ControlPacket::ProbeTail:
            if (_isReceivingData) {
                processProbeTail(move(controlPacket));
            }
            break;
        case ControlPacket::HandshakeRequest:
            if (_hasReceivedHandshakeACK) {
                // We're already in a state where we've received a handshake ack, so we are likely in a state
                // where the other end expired our connection. Let's reset.

#ifdef UDT_CONNECTION_DEBUG
                qCDebug(networking) << "Got handshake request, stopping SendQueue";
#endif
                stopSendQueue();
            }
            break;
    }
}

void Connection::processACK(ControlPacketPointer controlPacket) {
    // read the ACK sub-sequence number
    SequenceNumber currentACKSubSequenceNumber;
    controlPacket->readPrimitive(&currentACKSubSequenceNumber);

    // Check if we need send an ACK2 for this ACK
    // This will be the case if it has been longer than the sync interval OR
    // it looks like they haven't received our ACK2 for this ACK
    auto currentTime = p_high_resolution_clock::now();
    static p_high_resolution_clock::time_point lastACK2SendTime =
        p_high_resolution_clock::now() - std::chrono::microseconds(_synInterval);
    
    microseconds sinceLastACK2 = duration_cast<microseconds>(currentTime - lastACK2SendTime);
    
    if (_congestionControl->shouldACK2()
        && (sinceLastACK2.count() >= _synInterval || currentACKSubSequenceNumber == _lastSentACK2)) {
        // Send ACK2 packet
        sendACK2(currentACKSubSequenceNumber);
        
        lastACK2SendTime = p_high_resolution_clock::now();
    }
    
    // read the ACKed sequence number
    SequenceNumber ack;
    controlPacket->readPrimitive(&ack);
    
    // update the total count of received ACKs
    _stats.record(ConnectionStats::Stats::ReceivedACK);
    
    // validate that this isn't a BS ACK
    if (ack > getSendQueue().getCurrentSequenceNumber()) {
        // in UDT they specifically break the connection here - do we want to do anything?
        Q_ASSERT_X(false, "Connection::processACK", "ACK recieved higher than largest sent sequence number");
        return;
    }
    
    // read the RTT
    int32_t rtt;
    controlPacket->readPrimitive(&rtt);
    
    if (ack < _lastReceivedACK) {
        // this is an out of order ACK, bail
        return;
    }
    
    // this is a valid ACKed sequence number - update the flow window size and the last received ACK
    int32_t packedFlowWindow;
    controlPacket->readPrimitive(&packedFlowWindow);
    
    _flowWindowSize = packedFlowWindow;
    
    if (ack == _lastReceivedACK) {
        // processing an already received ACK, bail
        return;
    }
    
    _lastReceivedACK = ack;
    
    // ACK the send queue so it knows what was received
    getSendQueue().ack(ack);
    
    // update the RTT
    updateRTT(rtt);
    
    // write this RTT to stats
    _stats.recordRTT(rtt);
    
    // set the RTT for congestion control
    _congestionControl->setRTT(_rtt);
    
    if (controlPacket->bytesLeftToRead() > 0) {
        int32_t receiveRate, bandwidth;
        
        Q_ASSERT_X(controlPacket->bytesLeftToRead() == sizeof(receiveRate) + sizeof(bandwidth),
                   "Connection::processACK", "sync interval ACK packet does not contain expected data");
        
        controlPacket->readPrimitive(&receiveRate);
        controlPacket->readPrimitive(&bandwidth);
        
        // set the delivery rate and bandwidth for congestion control
        // these are calculated using an EWMA
        static const int EMWA_ALPHA_NUMERATOR = 8;
        
        // record these samples in connection stats
        _stats.recordSendRate(receiveRate);
        _stats.recordEstimatedBandwidth(bandwidth);
        
        _deliveryRate = (_deliveryRate * (EMWA_ALPHA_NUMERATOR - 1) + receiveRate) / EMWA_ALPHA_NUMERATOR;
        _bandwidth = (_bandwidth * (EMWA_ALPHA_NUMERATOR - 1) + bandwidth) / EMWA_ALPHA_NUMERATOR;
        
        _congestionControl->setReceiveRate(_deliveryRate);
        _congestionControl->setBandwidth(_bandwidth);
    }
    
    // give this ACK to the congestion control and update the send queue parameters
    updateCongestionControlAndSendQueue([this, ack, &controlPacket] {
        if (_congestionControl->onACK(ack, controlPacket->getReceiveTime())) {
            // the congestion control has told us it needs a fast re-transmit of ack + 1, add that now
            _sendQueue->fastRetransmit(ack + 1);
        }
    });
    
    _stats.record(ConnectionStats::Stats::ProcessedACK);
}

void Connection::processLightACK(ControlPacketPointer controlPacket) {
    // read the ACKed sequence number
    SequenceNumber ack;
    controlPacket->readPrimitive(&ack);
    
    // must be larger than the last received ACK to be processed
    if (ack > _lastReceivedACK) {
        // NOTE: the following makes sense in UDT where there is a dynamic receive buffer.
        // Since we have a receive buffer that is always of a default size, we don't use this light ACK to
        // drop the flow window size.
    
        // decrease the flow window size by the offset between the last received ACK and this ACK
        // _flowWindowSize -= seqoff(_lastReceivedACK, ack);
    
        // update the last received ACK to the this one
        _lastReceivedACK = ack;
        
        // send light ACK to the send queue
        getSendQueue().ack(ack);
    }
    
    _stats.record(ConnectionStats::Stats::ReceivedLightACK);
}

void Connection::processACK2(ControlPacketPointer controlPacket) {
    // pull the sub sequence number from the packet
    SequenceNumber subSequenceNumber;
    controlPacket->readPrimitive(&subSequenceNumber);

    // check if we had that subsequence number in our map
    auto it = std::find_if_not(_sentACKs.begin(), _sentACKs.end(), [&subSequenceNumber](const ACKListPair& pair){
        return pair.first < subSequenceNumber;
    });
    
    if (it != _sentACKs.end()) {
        if (it->first == subSequenceNumber){
            // update the RTT using the ACK window
            
            // calculate the RTT (time now - time ACK sent)
            auto now = p_high_resolution_clock::now();
            int rtt = duration_cast<microseconds>(now - it->second.second).count();
            
            updateRTT(rtt);
            // write this RTT to stats
            _stats.recordRTT(rtt);
            
            // set the RTT for congestion control
            _congestionControl->setRTT(_rtt);
            
            // update the last ACKed ACK
            if (it->second.first > _lastReceivedAcknowledgedACK) {
                _lastReceivedAcknowledgedACK = it->second.first;
            }
        } else if (it->first < subSequenceNumber) {
            Q_UNREACHABLE();
        }
    }
    
    // erase this sub-sequence number and anything below it now that we've gotten our timing information
    _sentACKs.erase(_sentACKs.begin(), it);
    
    _stats.record(ConnectionStats::Stats::ReceivedACK2);
}

void Connection::processNAK(ControlPacketPointer controlPacket) {
    // read the loss report
    SequenceNumber start, end;
    controlPacket->readPrimitive(&start);
    
    end = start;
    
    if (controlPacket->bytesLeftToRead() >= (qint64)sizeof(SequenceNumber)) {
        controlPacket->readPrimitive(&end);
    }
    
    // send that off to the send queue so it knows there was loss
    getSendQueue().nak(start, end);
    
    // give the loss to the congestion control object and update the send queue parameters
    updateCongestionControlAndSendQueue([this, start, end] {
        _congestionControl->onLoss(start, end);
    });
    
    _stats.record(ConnectionStats::Stats::ReceivedNAK);
}

void Connection::processHandshake(ControlPacketPointer controlPacket) {
    SequenceNumber initialSequenceNumber;
    controlPacket->readPrimitive(&initialSequenceNumber);
    
    if (!_hasReceivedHandshake || initialSequenceNumber != _initialReceiveSequenceNumber) {
        // server sent us a handshake - we need to assume this means state should be reset
        // as long as we haven't received a handshake yet or we have and we've received some data

#ifdef UDT_CONNECTION_DEBUG
        if (initialSequenceNumber != _initialReceiveSequenceNumber) {
            qCDebug(networking) << "Resetting receive state, received a new initial sequence number in handshake";
        }
#endif
        resetReceiveState();
        _initialReceiveSequenceNumber = initialSequenceNumber;
        _lastReceivedSequenceNumber = initialSequenceNumber - 1;
        _lastSentACK = initialSequenceNumber - 1;
    }

    _handshakeACK->reset();
    _handshakeACK->writePrimitive(initialSequenceNumber);
    _parentSocket->writeBasePacket(*_handshakeACK, _destination);
    
    // indicate that handshake has been received
    _hasReceivedHandshake = true;

    if (_didRequestHandshake) {
        emit receiverHandshakeRequestComplete(_destination);
        _didRequestHandshake = false;
    }
}

void Connection::processHandshakeACK(ControlPacketPointer controlPacket) {
    // if we've decided to clean up the send queue then this handshake ACK should be ignored, it's useless
    if (_sendQueue) {
        SequenceNumber initialSequenceNumber;
        controlPacket->readPrimitive(&initialSequenceNumber);

        // hand off this handshake ACK to the send queue so it knows it can start sending
        getSendQueue().handshakeACK(initialSequenceNumber);
        
        // indicate that handshake ACK was received
        _hasReceivedHandshakeACK = true;
    }
}

void Connection::processTimeoutNAK(ControlPacketPointer controlPacket) {
    // Override SendQueue's LossList with the timeout NAK list
    getSendQueue().overrideNAKListFromPacket(*controlPacket);
    
    // we don't tell the congestion control object there was loss here - this matches UDTs implementation
    // a possible improvement would be to tell it which new loss this timeout packet told us about
    
    _stats.record(ConnectionStats::Stats::ReceivedTimeoutNAK);
}

void Connection::processProbeTail(ControlPacketPointer controlPacket) {
    if (((uint32_t) _lastReceivedSequenceNumber & 0xF) == 0) {
        // this is the second packet in a probe set so we can estimate bandwidth
        // the sender sent this to us in lieu of sending new data (because they didn't have any)
        
#ifdef UDT_CONNECTION_DEBUG
        qCDebug(networking) << "Processing second packet of probe from control packet instead of data packet";
#endif
        
        _receiveWindow.onProbePair2Arrival();
        
        // mark that we processed a control packet for the second in the pair and we should not mark
        // the next data packet received
        _receivedControlProbeTail = true;
    }
}

void Connection::resetReceiveState() {
    
    // reset all SequenceNumber member variables back to default
    SequenceNumber defaultSequenceNumber;
    
    _lastReceivedSequenceNumber = defaultSequenceNumber;
    
    _lastReceivedAcknowledgedACK = defaultSequenceNumber;
    _currentACKSubSequenceNumber = defaultSequenceNumber;
    
    _lastSentACK = defaultSequenceNumber;
    
    // clear the sent ACKs
    _sentACKs.clear();
    
    // clear the loss list and _lastNAKTime
    _lossList.clear();
    _lastNAKTime = p_high_resolution_clock::now();
    
    // the _nakInterval need not be reset, that will happen on loss
    
    // clear sync variables
    _isReceivingData = false;
    _connectionStart = p_high_resolution_clock::now();
    
    _acksDuringSYN = 1;
    _lightACKsDuringSYN = 1;
    _packetsSinceACK = 0;
    
    // reset RTT to initial value
    resetRTT();
    
    // clear the intervals in the receive window
    _receiveWindow.reset();
    _receivedControlProbeTail = false;
    
    // clear any pending received messages
    for (auto& pendingMessage : _pendingReceivedMessages) {
        _parentSocket->messageFailed(this, pendingMessage.first);
    }
    _pendingReceivedMessages.clear();
}

void Connection::updateRTT(int rtt) {
    // This updates the RTT using exponential weighted moving average
    // This is the Jacobson's forumla for RTT estimation
    // http://www.mathcs.emory.edu/~cheung/Courses/455/Syllabus/7-transport/Jacobson-88.pdf
    
    // Estimated RTT = (1 - x)(estimatedRTT) + (x)(sampleRTT)
    // (where x = 0.125 via Jacobson)
    
    // Deviation  = (1 - x)(deviation) + x |sampleRTT - estimatedRTT|
    // (where x = 0.25 via Jacobson)
    
    static const int RTT_ESTIMATION_ALPHA_NUMERATOR = 8;
    static const int RTT_ESTIMATION_VARIANCE_ALPHA_NUMERATOR = 4;
   
    _rtt = (_rtt * (RTT_ESTIMATION_ALPHA_NUMERATOR - 1) + rtt) / RTT_ESTIMATION_ALPHA_NUMERATOR;
    
    _rttVariance = (_rttVariance * (RTT_ESTIMATION_VARIANCE_ALPHA_NUMERATOR - 1)
                    + abs(rtt - _rtt)) / RTT_ESTIMATION_VARIANCE_ALPHA_NUMERATOR;
}

int Connection::estimatedTimeout() const {
    return _congestionControl->_userDefinedRTO ? _congestionControl->_rto : _rtt + _rttVariance * 4;
}

void Connection::updateCongestionControlAndSendQueue(std::function<void ()> congestionCallback) {
    // update the last sent sequence number in congestion control
    _congestionControl->setSendCurrentSequenceNumber(getSendQueue().getCurrentSequenceNumber());
    
    // fire congestion control callback
    congestionCallback();
    
    auto& sendQueue = getSendQueue();
    
    // now that we've updated the congestion control, update the packet send period and flow window size
    sendQueue.setPacketSendPeriod(_congestionControl->_packetSendPeriod);
    sendQueue.setEstimatedTimeout(estimatedTimeout());
    sendQueue.setFlowWindowSize(std::min(_flowWindowSize, (int) _congestionControl->_congestionWindowSize));
    
    // record connection stats
    _stats.recordPacketSendPeriod(_congestionControl->_packetSendPeriod);
    _stats.recordCongestionWindowSize(_congestionControl->_congestionWindowSize);
}

void PendingReceivedMessage::enqueuePacket(std::unique_ptr<Packet> packet) {
    Q_ASSERT_X(packet->isPartOfMessage(),
               "PendingReceivedMessage::enqueuePacket",
               "called with a packet that is not part of a message");
    
    if (packet->getPacketPosition() == Packet::PacketPosition::LAST ||
        packet->getPacketPosition() == Packet::PacketPosition::ONLY) {
        _hasLastPacket = true;
        _numPackets = packet->getMessagePartNumber() + 1;
    }

    // Insert into the packets list in sorted order. Because we generally expect to receive packets in order, begin
    // searching from the end of the list.
    auto messagePartNumber = packet->getMessagePartNumber();
    auto it = std::find_if(_packets.rbegin(), _packets.rend(),
        [&](const std::unique_ptr<Packet>& value) { return messagePartNumber >= value->getMessagePartNumber(); });

    if (it != _packets.rend() && ((*it)->getMessagePartNumber() == messagePartNumber)) {
        qCDebug(networking) << "PendingReceivedMessage::enqueuePacket: This is a duplicate packet";
        return;
    }
    
    _packets.insert(it.base(), std::move(packet));
}

bool PendingReceivedMessage::hasAvailablePackets() const {
    return _packets.size() > 0
        && _nextPartNumber == _packets.front()->getMessagePartNumber();
}

std::unique_ptr<Packet> PendingReceivedMessage::removeNextPacket() {
    if (hasAvailablePackets()) {
        _nextPartNumber++;
        auto p = std::move(_packets.front());
        _packets.pop_front();
        return p;
    }
    return std::unique_ptr<Packet>();
}
