//
//  AudioClient.h
//  libraries/audio-client/src
//
//  Created by Stephen Birarda on 1/22/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AudioClient_h
#define hifi_AudioClient_h

#include <fstream>
#include <memory>
#include <vector>

#include <QtCore/QByteArray>
#include <QtCore/QElapsedTimer>
#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtMultimedia/QAudio>
#include <QtMultimedia/QAudioFormat>
#include <QtMultimedia/QAudioInput>

#include <AbstractAudioInterface.h>
#include <AudioBuffer.h>
#include <AudioEffectOptions.h>
#include <AudioFormat.h>
#include <AudioGain.h>
#include <AudioRingBuffer.h>
#include <AudioSourceTone.h>
#include <AudioSourceNoise.h>
#include <AudioStreamStats.h>

#include <DependencyManager.h>
#include <HifiSockAddr.h>
#include <NLPacket.h>
#include <MixedProcessedAudioStream.h>
#include <RingBufferHistory.h>
#include <SettingHandle.h>
#include <Sound.h>
#include <StDev.h>

#include "AudioIOStats.h"
#include "AudioNoiseGate.h"
#include "AudioSRC.h"
#include "AudioReverb.h"

#ifdef _WIN32
#pragma warning( push )
#pragma warning( disable : 4273 )
#pragma warning( disable : 4305 )
#endif

#ifdef _WIN32
#pragma warning( pop )
#endif

static const bool SEND_AUDIO_STATS = true;

static const int NUM_AUDIO_CHANNELS = 2;

static const int DEFAULT_AUDIO_OUTPUT_BUFFER_SIZE_FRAMES = 3;
static const int MIN_AUDIO_OUTPUT_BUFFER_SIZE_FRAMES = 1;
static const int MAX_AUDIO_OUTPUT_BUFFER_SIZE_FRAMES = 20;
#if defined(Q_OS_ANDROID) || defined(Q_OS_WIN)
    static const int DEFAULT_AUDIO_OUTPUT_STARVE_DETECTION_ENABLED = false;
#else
    static const int DEFAULT_AUDIO_OUTPUT_STARVE_DETECTION_ENABLED = true;
#endif
static const int DEFAULT_AUDIO_OUTPUT_STARVE_DETECTION_THRESHOLD = 3;
static const quint64 DEFAULT_AUDIO_OUTPUT_STARVE_DETECTION_PERIOD = 10 * 1000; // 10 Seconds

class QAudioInput;
class QAudioOutput;
class QIODevice;


class Transform;
class NLPacket;

class AudioClient : public AbstractAudioInterface, public Dependency {
    Q_OBJECT
    SINGLETON_DEPENDENCY
public:
    using AudioPositionGetter = std::function<glm::vec3()>;
    using AudioOrientationGetter = std::function<glm::quat()>;

    class AudioOutputIODevice : public QIODevice {
    public:
        AudioOutputIODevice(MixedProcessedAudioStream& receivedAudioStream, AudioClient* audio) :
            _receivedAudioStream(receivedAudioStream), _audio(audio), _unfulfilledReads(0) {};

        void start() { open(QIODevice::ReadOnly); }
        void stop() { close(); }
        qint64    readData(char * data, qint64 maxSize);
        qint64    writeData(const char * data, qint64 maxSize) { return 0; }

        int getRecentUnfulfilledReads() { int unfulfilledReads = _unfulfilledReads; _unfulfilledReads = 0; return unfulfilledReads; }
    private:
        MixedProcessedAudioStream& _receivedAudioStream;
        AudioClient* _audio;
        int _unfulfilledReads;
    };

    const MixedProcessedAudioStream& getReceivedAudioStream() const { return _receivedAudioStream; }
    MixedProcessedAudioStream& getReceivedAudioStream() { return _receivedAudioStream; }

    float getLastInputLoudness() const { return glm::max(_lastInputLoudness - _inputGate.getMeasuredFloor(), 0.0f); }

    float getTimeSinceLastClip() const { return _timeSinceLastClip; }
    float getAudioAverageInputLoudness() const { return _lastInputLoudness; }

    int getDesiredJitterBufferFrames() const { return _receivedAudioStream.getDesiredJitterBufferFrames(); }

    bool isMuted() { return _muted; }

    const AudioIOStats& getStats() const { return _stats; }

    float getInputRingBufferMsecsAvailable() const;
    float getAudioOutputMsecsUnplayed() const;

    int getOutputBufferSize() { return _outputBufferSizeFrames.get(); }

    bool getOutputStarveDetectionEnabled() { return _outputStarveDetectionEnabled.get(); }
    void setOutputStarveDetectionEnabled(bool enabled) { _outputStarveDetectionEnabled.set(enabled); }

    int getOutputStarveDetectionPeriod() { return _outputStarveDetectionPeriodMsec.get(); }
    void setOutputStarveDetectionPeriod(int msecs) { _outputStarveDetectionPeriodMsec.set(msecs); }

    int getOutputStarveDetectionThreshold() { return _outputStarveDetectionThreshold.get(); }
    void setOutputStarveDetectionThreshold(int threshold) { _outputStarveDetectionThreshold.set(threshold); }

    void setPositionGetter(AudioPositionGetter positionGetter) { _positionGetter = positionGetter; }
    void setOrientationGetter(AudioOrientationGetter orientationGetter) { _orientationGetter = orientationGetter; }

    static const float CALLBACK_ACCELERATOR_RATIO;

public slots:
    void start();
    void stop();

    void handleAudioEnvironmentDataPacket(QSharedPointer<ReceivedMessage> message);
    void handleAudioDataPacket(QSharedPointer<ReceivedMessage> message);
    void handleNoisyMutePacket(QSharedPointer<ReceivedMessage> message);
    void handleMuteEnvironmentPacket(QSharedPointer<ReceivedMessage> message);

    void sendDownstreamAudioStatsPacket() { _stats.sendDownstreamAudioStatsPacket(); }
    void handleAudioInput();
    void handleRecordedAudioInput(const QByteArray& audio);
    void reset();
    void audioMixerKilled();
    void toggleMute();

    virtual void enableAudioSourceInject(bool enable);
    virtual void selectAudioSourcePinkNoise();
    virtual void selectAudioSourceSine440();

    virtual void setIsStereoInput(bool stereo);

    void toggleAudioNoiseReduction() { _isNoiseGateEnabled = !_isNoiseGateEnabled; }

    void toggleLocalEcho() { _shouldEchoLocally = !_shouldEchoLocally; }
    void toggleServerEcho() { _shouldEchoToServer = !_shouldEchoToServer; }

    void processReceivedSamples(const QByteArray& inputBuffer, QByteArray& outputBuffer);
    void sendMuteEnvironmentPacket();

    void setOutputBufferSize(int numFrames);

    virtual bool outputLocalInjector(bool isStereo, AudioInjector* injector);

    bool switchInputToAudioDevice(const QString& inputDeviceName);
    bool switchOutputToAudioDevice(const QString& outputDeviceName);
    QString getDeviceName(QAudio::Mode mode) const { return (mode == QAudio::AudioInput) ?
                                                            _inputAudioDeviceName : _outputAudioDeviceName; }
    QString getDefaultDeviceName(QAudio::Mode mode);
    QVector<QString> getDeviceNames(QAudio::Mode mode);

    float getInputVolume() const { return (_audioInput) ? (float)_audioInput->volume() : 0.0f; }
    void setInputVolume(float volume) { if (_audioInput) _audioInput->setVolume(volume); }
    void setReverb(bool reverb);
    void setReverbOptions(const AudioEffectOptions* options);

    void outputNotify();

    void loadSettings();
    void saveSettings();

signals:
    bool muteToggled();
    void mutedByMixer();
    void inputReceived(const QByteArray& inputSamples);
    void outputBytesToNetwork(int numBytes);
    void inputBytesFromNetwork(int numBytes);

    void deviceChanged();

    void receivedFirstPacket();
    void disconnected();

    void audioFinished();

    void muteEnvironmentRequested(glm::vec3 position, float radius);

protected:
    AudioClient();
    ~AudioClient();

    virtual void customDeleter() {
        deleteLater();
    }

private:
    void outputFormatChanged();

    QByteArray firstInputFrame;
    QAudioInput* _audioInput;
    QAudioFormat _desiredInputFormat;
    QAudioFormat _inputFormat;
    QIODevice* _inputDevice;
    int _numInputCallbackBytes;
    int16_t _localProceduralSamples[AudioConstants::NETWORK_FRAME_SAMPLES_PER_CHANNEL];
    QAudioOutput* _audioOutput;
    QAudioFormat _desiredOutputFormat;
    QAudioFormat _outputFormat;
    int _outputFrameSize;
    int16_t _outputProcessingBuffer[AudioConstants::NETWORK_FRAME_SAMPLES_STEREO];
    int _numOutputCallbackBytes;
    QAudioOutput* _loopbackAudioOutput;
    QIODevice* _loopbackOutputDevice;
    AudioRingBuffer _inputRingBuffer;
    MixedProcessedAudioStream _receivedAudioStream;
    bool _isStereoInput;

    QString _inputAudioDeviceName;
    QString _outputAudioDeviceName;

    quint64 _outputStarveDetectionStartTimeMsec;
    int _outputStarveDetectionCount;

    Setting::Handle<int> _outputBufferSizeFrames;
    Setting::Handle<bool> _outputStarveDetectionEnabled;
    Setting::Handle<int> _outputStarveDetectionPeriodMsec;
     // Maximum number of starves per _outputStarveDetectionPeriod before increasing buffer size
    Setting::Handle<int> _outputStarveDetectionThreshold;

    StDev _stdev;
    QElapsedTimer _timeSinceLastReceived;
    float _averagedLatency;
    float _lastInputLoudness;
    float _timeSinceLastClip;
    int _totalInputAudioSamples;

    bool _muted;
    bool _shouldEchoLocally;
    bool _shouldEchoToServer;
    bool _isNoiseGateEnabled;
    bool _audioSourceInjectEnabled;

    bool _reverb;
    AudioEffectOptions _scriptReverbOptions;
    AudioEffectOptions _zoneReverbOptions;
    AudioEffectOptions* _reverbOptions;
    AudioReverb _sourceReverb { AudioConstants::SAMPLE_RATE };
    AudioReverb _listenerReverb { AudioConstants::SAMPLE_RATE };

    // possible streams needed for resample
    AudioSRC* _inputToNetworkResampler;
    AudioSRC* _networkToOutputResampler;
    AudioSRC* _loopbackResampler;

    // Adds Reverb
    void configureReverb();
    void updateReverbOptions();

    void handleLocalEchoAndReverb(QByteArray& inputByteArray);

    bool switchInputToAudioDevice(const QAudioDeviceInfo& inputDeviceInfo);
    bool switchOutputToAudioDevice(const QAudioDeviceInfo& outputDeviceInfo);

    // Callback acceleration dependent calculations
    int calculateNumberOfInputCallbackBytes(const QAudioFormat& format) const;
    int calculateNumberOfFrameSamples(int numBytes) const;
    float calculateDeviceToNetworkInputRatio() const;

    // Input framebuffer
    AudioBufferFloat32 _inputFrameBuffer;

    // Input gain
    AudioGain _inputGain;

    // Post tone/pink noise generator gain
    AudioGain _sourceGain;

    // Pink noise source
    bool _noiseSourceEnabled;
    AudioSourcePinkNoise _noiseSource;

    // Tone source
    bool _toneSourceEnabled;
    AudioSourceTone _toneSource;

    quint16 _outgoingAvatarAudioSequenceNumber;

    AudioOutputIODevice _audioOutputIODevice;

    AudioIOStats _stats;

    AudioNoiseGate _inputGate;

    AudioPositionGetter _positionGetter;
    AudioOrientationGetter _orientationGetter;

    QVector<QString> _inputDevices;
    QVector<QString> _outputDevices;
    void checkDevices();

    bool _hasReceivedFirstPacket = false;
};


#endif // hifi_AudioClient_h
