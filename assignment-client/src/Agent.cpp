//
//  Agent.cpp
//  assignment-client/src
//
//  Created by Stephen Birarda on 7/1/13.
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QtCore/QCoreApplication>
#include <QtCore/QEventLoop>
#include <QtCore/QStandardPaths>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>

#include <AssetClient.h>
#include <AvatarHashMap.h>
#include <AudioInjectorManager.h>
#include <AssetClient.h>
#include <MessagesClient.h>
#include <NetworkAccessManager.h>
#include <NodeList.h>
#include <udt/PacketHeaders.h>
#include <ResourceCache.h>
#include <ScriptCache.h>
#include <SoundCache.h>
#include <UUID.h>

#include <recording/Deck.h>
#include <recording/Recorder.h>
#include <recording/Frame.h>

#include <WebSocketServerClass.h>
#include <EntityScriptingInterface.h> // TODO: consider moving to scriptengine.h

#include "avatars/ScriptableAvatar.h"
#include "entities/AssignmentParentFinder.h"
#include "RecordingScriptingInterface.h"
#include "AbstractAudioInterface.h"

#include "Agent.h"

static const int RECEIVED_AUDIO_STREAM_CAPACITY_FRAMES = 10;

Agent::Agent(ReceivedMessage& message) :
    ThreadedAssignment(message),
    _entityEditSender(),
    _receivedAudioStream(AudioConstants::NETWORK_FRAME_SAMPLES_STEREO, RECEIVED_AUDIO_STREAM_CAPACITY_FRAMES,
        InboundAudioStream::Settings(0, false, RECEIVED_AUDIO_STREAM_CAPACITY_FRAMES, false,
        DEFAULT_WINDOW_STARVE_THRESHOLD, DEFAULT_WINDOW_SECONDS_FOR_DESIRED_CALC_ON_TOO_MANY_STARVES,
        DEFAULT_WINDOW_SECONDS_FOR_DESIRED_REDUCTION, false))
{
    DependencyManager::get<EntityScriptingInterface>()->setPacketSender(&_entityEditSender);

    auto assetClient = DependencyManager::set<AssetClient>();

    QThread* assetThread = new QThread;
    assetThread->setObjectName("Asset Thread");
    assetClient->moveToThread(assetThread);
    connect(assetThread, &QThread::started, assetClient.data(), &AssetClient::init);
    assetThread->start();

    DependencyManager::registerInheritance<SpatialParentFinder, AssignmentParentFinder>();

    DependencyManager::set<ResourceCacheSharedItems>();
    DependencyManager::set<SoundCache>();
    DependencyManager::set<AudioInjectorManager>();
    DependencyManager::set<recording::Deck>();
    DependencyManager::set<recording::Recorder>();
    DependencyManager::set<RecordingScriptingInterface>();

    auto& packetReceiver = DependencyManager::get<NodeList>()->getPacketReceiver();

    packetReceiver.registerListenerForTypes(
        { PacketType::MixedAudio, PacketType::SilentAudioFrame },
        this, "handleAudioPacket");
    packetReceiver.registerListenerForTypes(
        { PacketType::OctreeStats, PacketType::EntityData, PacketType::EntityErase },
        this, "handleOctreePacket");
    packetReceiver.registerListener(PacketType::Jurisdiction, this, "handleJurisdictionPacket");
}

void Agent::handleOctreePacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode) {
    auto packetType = message->getType();

    if (packetType == PacketType::OctreeStats) {

        int statsMessageLength = OctreeHeadlessViewer::parseOctreeStats(message, senderNode);
        if (message->getSize() > statsMessageLength) {
            // pull out the piggybacked packet and create a new QSharedPointer<NLPacket> for it
            int piggyBackedSizeWithHeader = message->getSize() - statsMessageLength;
            
            auto buffer = std::unique_ptr<char[]>(new char[piggyBackedSizeWithHeader]);
            memcpy(buffer.get(), message->getRawMessage() + statsMessageLength, piggyBackedSizeWithHeader);

            auto newPacket = NLPacket::fromReceivedPacket(std::move(buffer), piggyBackedSizeWithHeader, message->getSenderSockAddr());
            message = QSharedPointer<ReceivedMessage>::create(*newPacket);
        } else {
            return; // bail since no piggyback data
        }

        packetType = message->getType();
    } // fall through to piggyback message

    if (packetType == PacketType::EntityData) {
        _entityViewer.processDatagram(*message, senderNode);
    } else if (packetType == PacketType::EntityErase) {
        _entityViewer.processEraseMessage(*message, senderNode);
    }
}

void Agent::handleJurisdictionPacket(QSharedPointer<ReceivedMessage> message, SharedNodePointer senderNode) {
    NodeType_t nodeType;
    message->peekPrimitive(&nodeType);

    // PacketType_JURISDICTION, first byte is the node type...
    if (nodeType == NodeType::EntityServer) {
        DependencyManager::get<EntityScriptingInterface>()->getJurisdictionListener()->
            queueReceivedPacket(message, senderNode);
    }
}

void Agent::handleAudioPacket(QSharedPointer<ReceivedMessage> message) {
    _receivedAudioStream.parseData(*message);

    _lastReceivedAudioLoudness = _receivedAudioStream.getNextOutputFrameLoudness();

    _receivedAudioStream.clearBuffer();
}

const QString AGENT_LOGGING_NAME = "agent";

void Agent::run() {

    // make sure we request our script once the agent connects to the domain
    auto nodeList = DependencyManager::get<NodeList>();

    connect(&nodeList->getDomainHandler(), &DomainHandler::connectedToDomain, this, &Agent::requestScript);

    ThreadedAssignment::commonInit(AGENT_LOGGING_NAME, NodeType::Agent);

    // Setup MessagesClient
    auto messagesClient = DependencyManager::set<MessagesClient>();
    QThread* messagesThread = new QThread;
    messagesThread->setObjectName("Messages Client Thread");
    messagesClient->moveToThread(messagesThread);
    connect(messagesThread, &QThread::started, messagesClient.data(), &MessagesClient::init);
    messagesThread->start();

    nodeList->addSetOfNodeTypesToNodeInterestSet({
        NodeType::AudioMixer, NodeType::AvatarMixer, NodeType::EntityServer, NodeType::MessagesMixer, NodeType::AssetServer
    });
}

void Agent::requestScript() {
    auto nodeList = DependencyManager::get<NodeList>();
    disconnect(&nodeList->getDomainHandler(), &DomainHandler::connectedToDomain, this, &Agent::requestScript);

    // figure out the URL for the script for this agent assignment
    QUrl scriptURL;
    if (_payload.isEmpty())  {
        scriptURL = QUrl(QString("http://%1:%2/assignment/%3/")
                         .arg(nodeList->getDomainHandler().getIP().toString())
                         .arg(DOMAIN_SERVER_HTTP_PORT)
                         .arg(uuidStringWithoutCurlyBraces(nodeList->getSessionUUID())));
    } else {
        scriptURL = QUrl(_payload);
    }

    // setup a network access manager and
    QNetworkAccessManager& networkAccessManager = NetworkAccessManager::getInstance();

    QNetworkDiskCache* cache = new QNetworkDiskCache();
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
    cache->setCacheDirectory(!cachePath.isEmpty() ? cachePath : "agentCache");
    networkAccessManager.setCache(cache);

    QNetworkRequest networkRequest = QNetworkRequest(scriptURL);
    networkRequest.setHeader(QNetworkRequest::UserAgentHeader, HIGH_FIDELITY_USER_AGENT);

    // setup a timeout for script request
    static const int SCRIPT_TIMEOUT_MS = 10000;
    _scriptRequestTimeout = new QTimer(this);
    connect(_scriptRequestTimeout, &QTimer::timeout, this, &Agent::scriptRequestFinished);
    _scriptRequestTimeout->start(SCRIPT_TIMEOUT_MS);

    qDebug() << "Downloading script at" << scriptURL.toString();
    QNetworkReply* reply = networkAccessManager.get(networkRequest);
    connect(reply, &QNetworkReply::finished, this, &Agent::scriptRequestFinished);
}

void Agent::scriptRequestFinished() {
    auto reply = qobject_cast<QNetworkReply*>(sender());

    _scriptRequestTimeout->stop();

    if (reply && reply->error() == QNetworkReply::NoError) {
        _scriptContents = reply->readAll();
        qDebug() << "Downloaded script:" << _scriptContents;

        // we could just call executeScript directly - we use a QueuedConnection to allow scriptRequestFinished
        // to return before calling executeScript
        QMetaObject::invokeMethod(this, "executeScript", Qt::QueuedConnection);
    } else {
        if (reply) {
            qDebug() << "Failed to download script at" << reply->url().toString() << " - bailing on assignment.";
            qDebug() << "QNetworkReply error was" << reply->errorString();
        } else {
            qDebug() << "Failed to download script - request timed out. Bailing on assignment.";
        }

        setFinished(true);
    }

    reply->deleteLater();
}

void Agent::executeScript() {
    _scriptEngine = std::unique_ptr<ScriptEngine>(new ScriptEngine(_scriptContents, _payload));
    _scriptEngine->setParent(this); // be the parent of the script engine so it gets moved when we do

    // setup an Avatar for the script to use
    auto scriptedAvatar = DependencyManager::get<ScriptableAvatar>();
    connect(_scriptEngine.get(), SIGNAL(update(float)), scriptedAvatar.data(), SLOT(update(float)), Qt::ConnectionType::QueuedConnection);
    scriptedAvatar->setForceFaceTrackerConnected(true);

    // call model URL setters with empty URLs so our avatar, if user, will have the default models
    scriptedAvatar->setFaceModelURL(QUrl());
    scriptedAvatar->setSkeletonModelURL(QUrl());

    // give this AvatarData object to the script engine
    _scriptEngine->registerGlobalObject("Avatar", scriptedAvatar.data());


    using namespace recording;
    static const FrameType AVATAR_FRAME_TYPE = Frame::registerFrameType(AvatarData::FRAME_NAME);
    // FIXME how to deal with driving multiple avatars locally?
    Frame::registerFrameHandler(AVATAR_FRAME_TYPE, [this, scriptedAvatar](Frame::ConstPointer frame) {
        AvatarData::fromFrame(frame->data, *scriptedAvatar);
    });

    using namespace recording;
    static const FrameType AUDIO_FRAME_TYPE = Frame::registerFrameType(AudioConstants::getAudioFrameName());
    Frame::registerFrameHandler(AUDIO_FRAME_TYPE, [this, &scriptedAvatar](Frame::ConstPointer frame) {
        const QByteArray& audio = frame->data;
        static quint16 audioSequenceNumber{ 0 };
        Transform audioTransform;
        audioTransform.setTranslation(scriptedAvatar->getPosition());
        audioTransform.setRotation(scriptedAvatar->getOrientation());
        AbstractAudioInterface::emitAudioPacket(audio.data(), audio.size(), audioSequenceNumber, audioTransform, PacketType::MicrophoneAudioNoEcho);
    });

    auto avatarHashMap = DependencyManager::set<AvatarHashMap>();
    _scriptEngine->registerGlobalObject("AvatarList", avatarHashMap.data());

    auto& packetReceiver = DependencyManager::get<NodeList>()->getPacketReceiver();
    packetReceiver.registerListener(PacketType::BulkAvatarData, avatarHashMap.data(), "processAvatarDataPacket");
    packetReceiver.registerListener(PacketType::KillAvatar, avatarHashMap.data(), "processKillAvatar");
    packetReceiver.registerListener(PacketType::AvatarIdentity, avatarHashMap.data(), "processAvatarIdentityPacket");
    packetReceiver.registerListener(PacketType::AvatarBillboard, avatarHashMap.data(), "processAvatarBillboardPacket");

    // register ourselves to the script engine
    _scriptEngine->registerGlobalObject("Agent", this);

    // FIXME -we shouldn't be calling this directly, it's normally called by run(), not sure why
    // viewers would need this called.
    //_scriptEngine->init(); // must be done before we set up the viewers

    _scriptEngine->registerGlobalObject("SoundCache", DependencyManager::get<SoundCache>().data());

    QScriptValue webSocketServerConstructorValue = _scriptEngine->newFunction(WebSocketServerClass::constructor);
    _scriptEngine->globalObject().setProperty("WebSocketServer", webSocketServerConstructorValue);

    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();

    _scriptEngine->registerGlobalObject("EntityViewer", &_entityViewer);

    // we need to make sure that init has been called for our EntityScriptingInterface
    // so that it actually has a jurisdiction listener when we ask it for it next
    entityScriptingInterface->init();
    _entityViewer.setJurisdictionListener(entityScriptingInterface->getJurisdictionListener());

    _entityViewer.init();

    entityScriptingInterface->setEntityTree(_entityViewer.getTree());

    DependencyManager::set<AssignmentParentFinder>(_entityViewer.getTree());

    // wire up our additional agent related processing to the update signal
    QObject::connect(_scriptEngine.get(), &ScriptEngine::update, this, &Agent::processAgentAvatarAndAudio);

    _scriptEngine->run();

    Frame::clearFrameHandler(AUDIO_FRAME_TYPE);
    Frame::clearFrameHandler(AVATAR_FRAME_TYPE);

    setFinished(true);
}

QUuid Agent::getSessionUUID() const {
    return DependencyManager::get<NodeList>()->getSessionUUID();
}


void Agent::setIsAvatar(bool isAvatar) {
    _isAvatar = isAvatar;

    if (_isAvatar && !_avatarIdentityTimer) {
        // set up the avatar timers
        _avatarIdentityTimer = new QTimer(this);
        _avatarBillboardTimer = new QTimer(this);

        // connect our slot
        connect(_avatarIdentityTimer, &QTimer::timeout, this, &Agent::sendAvatarIdentityPacket);
        connect(_avatarBillboardTimer, &QTimer::timeout, this, &Agent::sendAvatarBillboardPacket);

        // start the timers
        _avatarIdentityTimer->start(AVATAR_IDENTITY_PACKET_SEND_INTERVAL_MSECS);
        _avatarBillboardTimer->start(AVATAR_BILLBOARD_PACKET_SEND_INTERVAL_MSECS);
    }

    if (!_isAvatar) {

        if (_avatarIdentityTimer) {
            _avatarIdentityTimer->stop();
            delete _avatarIdentityTimer;
            _avatarIdentityTimer = nullptr;
        }

        if (_avatarBillboardTimer) {
            _avatarBillboardTimer->stop();
            delete _avatarBillboardTimer;
            _avatarBillboardTimer = nullptr;
        }
    }
}

void Agent::sendAvatarIdentityPacket() {
    if (_isAvatar) {
        auto scriptedAvatar = DependencyManager::get<ScriptableAvatar>();
        scriptedAvatar->sendIdentityPacket();
    }
}

void Agent::sendAvatarBillboardPacket() {
    if (_isAvatar) {
        auto scriptedAvatar = DependencyManager::get<ScriptableAvatar>();
        scriptedAvatar->sendBillboardPacket();
    }
}


void Agent::processAgentAvatarAndAudio(float deltaTime) {
    if (!_scriptEngine->isFinished() && _isAvatar) {
        auto scriptedAvatar = DependencyManager::get<ScriptableAvatar>();
        const int SCRIPT_AUDIO_BUFFER_SAMPLES = floor(((SCRIPT_DATA_CALLBACK_USECS * AudioConstants::SAMPLE_RATE)
            / (1000 * 1000)) + 0.5);
        const int SCRIPT_AUDIO_BUFFER_BYTES = SCRIPT_AUDIO_BUFFER_SAMPLES * sizeof(int16_t);

        QByteArray avatarByteArray = scriptedAvatar->toByteArray(true, randFloat() < AVATAR_SEND_FULL_UPDATE_RATIO);
        scriptedAvatar->doneEncoding(true);

        static AvatarDataSequenceNumber sequenceNumber = 0;
        auto avatarPacket = NLPacket::create(PacketType::AvatarData, avatarByteArray.size() + sizeof(sequenceNumber));
        avatarPacket->writePrimitive(sequenceNumber++);

        avatarPacket->write(avatarByteArray);

        auto nodeList = DependencyManager::get<NodeList>();

        nodeList->broadcastToNodes(std::move(avatarPacket), NodeSet() << NodeType::AvatarMixer);

        if (_isListeningToAudioStream || _avatarSound) {
            // if we have an avatar audio stream then send it out to our audio-mixer
            bool silentFrame = true;

            int16_t numAvailableSamples = SCRIPT_AUDIO_BUFFER_SAMPLES;
            const int16_t* nextSoundOutput = NULL;

            if (_avatarSound) {

                const QByteArray& soundByteArray = _avatarSound->getByteArray();
                nextSoundOutput = reinterpret_cast<const int16_t*>(soundByteArray.data()
                    + _numAvatarSoundSentBytes);

                int numAvailableBytes = (soundByteArray.size() - _numAvatarSoundSentBytes) > SCRIPT_AUDIO_BUFFER_BYTES
                    ? SCRIPT_AUDIO_BUFFER_BYTES
                    : soundByteArray.size() - _numAvatarSoundSentBytes;
                numAvailableSamples = (int16_t)numAvailableBytes / sizeof(int16_t);


                // check if the all of the _numAvatarAudioBufferSamples to be sent are silence
                for (int i = 0; i < numAvailableSamples; ++i) {
                    if (nextSoundOutput[i] != 0) {
                        silentFrame = false;
                        break;
                    }
                }

                _numAvatarSoundSentBytes += numAvailableBytes;
                if (_numAvatarSoundSentBytes == soundByteArray.size()) {
                    // we're done with this sound object - so set our pointer back to NULL
                    // and our sent bytes back to zero
                    _avatarSound = NULL;
                    _numAvatarSoundSentBytes = 0;
                }
            }

            auto audioPacket = NLPacket::create(silentFrame
                ? PacketType::SilentAudioFrame
                : PacketType::MicrophoneAudioNoEcho);

            // seek past the sequence number, will be packed when destination node is known
            audioPacket->seek(sizeof(quint16));

            if (silentFrame) {
                if (!_isListeningToAudioStream) {
                    // if we have a silent frame and we're not listening then just send nothing and break out of here
                    return;
                }

                // write the number of silent samples so the audio-mixer can uphold timing
                audioPacket->writePrimitive(SCRIPT_AUDIO_BUFFER_SAMPLES);

                // use the orientation and position of this avatar for the source of this audio
                audioPacket->writePrimitive(scriptedAvatar->getPosition());
                glm::quat headOrientation = scriptedAvatar->getHeadOrientation();
                audioPacket->writePrimitive(headOrientation);

            } else if (nextSoundOutput) {
                // assume scripted avatar audio is mono and set channel flag to zero
                audioPacket->writePrimitive((quint8)0);

                // use the orientation and position of this avatar for the source of this audio
                audioPacket->writePrimitive(scriptedAvatar->getPosition());
                glm::quat headOrientation = scriptedAvatar->getHeadOrientation();
                audioPacket->writePrimitive(headOrientation);

                // write the raw audio data
                audioPacket->write(reinterpret_cast<const char*>(nextSoundOutput), numAvailableSamples * sizeof(int16_t));
            }

            // write audio packet to AudioMixer nodes
            auto nodeList = DependencyManager::get<NodeList>();
            nodeList->eachNode([this, &nodeList, &audioPacket](const SharedNodePointer& node){
                // only send to nodes of type AudioMixer
                if (node->getType() == NodeType::AudioMixer) {
                    // pack sequence number
                    quint16 sequence = _outgoingScriptAudioSequenceNumbers[node->getUUID()]++;
                    audioPacket->seek(0);
                    audioPacket->writePrimitive(sequence);

                    // send audio packet
                    nodeList->sendUnreliablePacket(*audioPacket, *node);
                }
            });
        }
    }
}

void Agent::aboutToFinish() {
    setIsAvatar(false);// will stop timers for sending billboards and identity packets

    if (_scriptEngine) {
        _scriptEngine->stop();
    }

    // our entity tree is going to go away so tell that to the EntityScriptingInterface
    DependencyManager::get<EntityScriptingInterface>()->setEntityTree(nullptr);

    // cleanup the AssetClient thread
    QThread* assetThread = DependencyManager::get<AssetClient>()->thread();
    DependencyManager::destroy<AssetClient>();
    assetThread->quit();
    assetThread->wait();
    
    // cleanup the AudioInjectorManager (and any still running injectors)
    DependencyManager::destroy<AudioInjectorManager>();
}
