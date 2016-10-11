//
//  InjectedAudioStream.h
//  libraries/audio/src
//
//  Created by Stephen Birarda on 6/5/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_InjectedAudioStream_h
#define hifi_InjectedAudioStream_h

#include <QtCore/QUuid>

#include "PositionalAudioStream.h"

class InjectedAudioStream : public PositionalAudioStream {
public:
    InjectedAudioStream(const QUuid& streamIdentifier, bool isStereo, int numStaticJitterFrames = -1);

    float getRadius() const { return _radius; }
    float getAttenuationRatio() const { return _attenuationRatio; }

    virtual const QUuid& getStreamIdentifier() const override { return _streamIdentifier; }

private:
    // disallow copying of InjectedAudioStream objects
    InjectedAudioStream(const InjectedAudioStream&);
    InjectedAudioStream& operator= (const InjectedAudioStream&);

    AudioStreamStats getAudioStreamStats() const override;
    int parseStreamProperties(PacketType type, const QByteArray& packetAfterSeqNum, int& numAudioSamples) override;

    const QUuid _streamIdentifier;
    float _radius;
    float _attenuationRatio;
};

#endif // hifi_InjectedAudioStream_h
