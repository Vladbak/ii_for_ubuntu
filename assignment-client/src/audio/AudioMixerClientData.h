//
//  AudioMixerClientData.h
//  assignment-client/src/audio
//
//  Created by Stephen Birarda on 10/18/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AudioMixerClientData_h
#define hifi_AudioMixerClientData_h

#include <QtCore/QJsonObject>

#include <AABox.h>
#include <AudioHRTF.h>
#include <UUIDHasher.h>

#include "PositionalAudioStream.h"
#include "AvatarAudioStream.h"

class AudioMixerClientData : public NodeData {
    Q_OBJECT
public:
    AudioMixerClientData(const QUuid& nodeID);

    using SharedStreamPointer = std::shared_ptr<PositionalAudioStream>;
    using AudioStreamMap = std::unordered_map<QUuid, SharedStreamPointer>;

    // locks the mutex to make a copy
    AudioStreamMap getAudioStreams() { QReadLocker readLock { &_streamsLock }; return _audioStreams; }
    AvatarAudioStream* getAvatarAudioStream();

    // the following methods should be called from the AudioMixer assignment thread ONLY
    // they are not thread-safe

    // returns a new or existing HRTF object for the given stream from the given node
    AudioHRTF& hrtfForStream(const QUuid& nodeID, const QUuid& streamID = QUuid()) { return _nodeSourcesHRTFMap[nodeID][streamID]; }

    // remove HRTFs for all sources from this node
    void removeHRTFsForNode(const QUuid& nodeID) { _nodeSourcesHRTFMap.erase(nodeID); }

    // removes an AudioHRTF object for a given stream
    void removeHRTFForStream(const QUuid& nodeID, const QUuid& streamID = QUuid());
    
    int parseData(ReceivedMessage& message);

    void checkBuffersBeforeFrameSend();

    void removeDeadInjectedStreams();

    QJsonObject getAudioStreamStats();
    
    void sendAudioStreamStatsPackets(const SharedNodePointer& destinationNode);
    
    void incrementOutgoingMixedAudioSequenceNumber() { _outgoingMixedAudioSequenceNumber++; }
    quint16 getOutgoingSequenceNumber() const { return _outgoingMixedAudioSequenceNumber; }

signals:
    void injectorStreamFinished(const QUuid& streamIdentifier);

private:
    QReadWriteLock _streamsLock;
    AudioStreamMap _audioStreams; // microphone stream from avatar is stored under key of null UUID

    using HRTFMap = std::unordered_map<QUuid, AudioHRTF>;
    using NodeSourcesHRTFMap = std::unordered_map<QUuid, HRTFMap>;
    NodeSourcesHRTFMap _nodeSourcesHRTFMap;

    quint16 _outgoingMixedAudioSequenceNumber;

    AudioStreamStats _downstreamAudioStreamStats;
};

#endif // hifi_AudioMixerClientData_h
