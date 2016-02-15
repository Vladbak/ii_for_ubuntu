//
//  NeuronPlugin.cpp
//  input-plugins/src/input-plugins
//
//  Created by Anthony Thibault on 12/18/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "NeuronPlugin.h"

#include <controllers/UserInputMapper.h>
#include <QLoggingCategory>
#include <PathUtils.h>
#include <DebugDraw.h>
#include <cassert>
#include <NumericalConstants.h>
#include <StreamUtils.h>

Q_DECLARE_LOGGING_CATEGORY(inputplugins)
Q_LOGGING_CATEGORY(inputplugins, "hifi.inputplugins")

#define __OS_XUN__ 1
#define BOOL int

#ifdef HAVE_NEURON
#include <NeuronDataReader.h>
#endif

const QString NeuronPlugin::NAME = "Neuron";
const QString NeuronPlugin::NEURON_ID_STRING = "Perception Neuron";

// indices of joints of the Neuron standard skeleton.
// This is 'almost' the same as the High Fidelity standard skeleton.
// It is missing a thumb joint.
enum NeuronJointIndex {
    Hips = 0,
    RightUpLeg,
    RightLeg,
    RightFoot,
    LeftUpLeg,
    LeftLeg,
    LeftFoot,
    Spine,
    Spine1,
    Spine2,
    Spine3,
    Neck,
    Head,
    RightShoulder,
    RightArm,
    RightForeArm,
    RightHand,
    RightHandThumb1,
    RightHandThumb2,
    RightHandThumb3,
    RightInHandIndex,
    RightHandIndex1,
    RightHandIndex2,
    RightHandIndex3,
    RightInHandMiddle,
    RightHandMiddle1,
    RightHandMiddle2,
    RightHandMiddle3,
    RightInHandRing,
    RightHandRing1,
    RightHandRing2,
    RightHandRing3,
    RightInHandPinky,
    RightHandPinky1,
    RightHandPinky2,
    RightHandPinky3,
    LeftShoulder,
    LeftArm,
    LeftForeArm,
    LeftHand,
    LeftHandThumb1,
    LeftHandThumb2,
    LeftHandThumb3,
    LeftInHandIndex,
    LeftHandIndex1,
    LeftHandIndex2,
    LeftHandIndex3,
    LeftInHandMiddle,
    LeftHandMiddle1,
    LeftHandMiddle2,
    LeftHandMiddle3,
    LeftInHandRing,
    LeftHandRing1,
    LeftHandRing2,
    LeftHandRing3,
    LeftInHandPinky,
    LeftHandPinky1,
    LeftHandPinky2,
    LeftHandPinky3,
    Size
};

// Almost a direct mapping except for LEFT_HAND_THUMB1 and RIGHT_HAND_THUMB1,
// which are not present in the Neuron standard skeleton.
static controller::StandardPoseChannel neuronJointIndexToPoseIndexMap[NeuronJointIndex::Size] = {
    controller::HIPS,
    controller::RIGHT_UP_LEG,
    controller::RIGHT_LEG,
    controller::RIGHT_FOOT,
    controller::LEFT_UP_LEG,
    controller::LEFT_LEG,
    controller::LEFT_FOOT,
    controller::SPINE,
    controller::SPINE1,
    controller::SPINE2,
    controller::SPINE3,
    controller::NECK,
    controller::HEAD,
    controller::RIGHT_SHOULDER,
    controller::RIGHT_ARM,
    controller::RIGHT_FORE_ARM,
    controller::RIGHT_HAND,
    controller::RIGHT_HAND_THUMB2,
    controller::RIGHT_HAND_THUMB3,
    controller::RIGHT_HAND_THUMB4,
    controller::RIGHT_HAND_INDEX1,
    controller::RIGHT_HAND_INDEX2,
    controller::RIGHT_HAND_INDEX3,
    controller::RIGHT_HAND_INDEX4,
    controller::RIGHT_HAND_MIDDLE1,
    controller::RIGHT_HAND_MIDDLE2,
    controller::RIGHT_HAND_MIDDLE3,
    controller::RIGHT_HAND_MIDDLE4,
    controller::RIGHT_HAND_RING1,
    controller::RIGHT_HAND_RING2,
    controller::RIGHT_HAND_RING3,
    controller::RIGHT_HAND_RING4,
    controller::RIGHT_HAND_PINKY1,
    controller::RIGHT_HAND_PINKY2,
    controller::RIGHT_HAND_PINKY3,
    controller::RIGHT_HAND_PINKY4,
    controller::LEFT_SHOULDER,
    controller::LEFT_ARM,
    controller::LEFT_FORE_ARM,
    controller::LEFT_HAND,
    controller::LEFT_HAND_THUMB2,
    controller::LEFT_HAND_THUMB3,
    controller::LEFT_HAND_THUMB4,
    controller::LEFT_HAND_INDEX1,
    controller::LEFT_HAND_INDEX2,
    controller::LEFT_HAND_INDEX3,
    controller::LEFT_HAND_INDEX4,
    controller::LEFT_HAND_MIDDLE1,
    controller::LEFT_HAND_MIDDLE2,
    controller::LEFT_HAND_MIDDLE3,
    controller::LEFT_HAND_MIDDLE4,
    controller::LEFT_HAND_RING1,
    controller::LEFT_HAND_RING2,
    controller::LEFT_HAND_RING3,
    controller::LEFT_HAND_RING4,
    controller::LEFT_HAND_PINKY1,
    controller::LEFT_HAND_PINKY2,
    controller::LEFT_HAND_PINKY3,
    controller::LEFT_HAND_PINKY4
};

// in rig frame
static glm::vec3 rightHandThumb1DefaultAbsTranslation(-2.155500650405884, -0.7610001564025879, 2.685631036758423);
static glm::vec3 leftHandThumb1DefaultAbsTranslation(2.1555817127227783, -0.7603635787963867, 2.6856393814086914);

// default translations (cm)
static glm::vec3 neuronJointTranslations[NeuronJointIndex::Size] = {
    {131.901, 95.6602, -27.9815},
    {-9.55907, -1.58772, 0.0760284},
    {0.0144232, -41.4683, -0.105322},
    {1.59348, -41.5875, -0.557237},
    {9.72077, -1.68926, -0.280643},
    {0.0886684, -43.1586, -0.0111596},
    {-2.98473, -44.0517, 0.0694456},
    {0.110967, 16.3959, 0.140463},
    {0.0500451, 10.0238, 0.0731921},
    {0.061568, 10.4352, 0.0583075},
    {0.0500606, 10.0217, 0.0711083},
    {0.0317731, 10.7176, 0.0779325},
    {-0.0204253, 9.71067, 0.131734},
    {-3.24245, 7.13584, 0.185638},
    {-13.0885, -0.0877601, 0.176065},
    {-27.2674, 0.0688724, 0.0272146},
    {-26.7673, 0.0301916, 0.0102847},
    {-2.56017, 0.195537, 3.20968},
    {-3.78796, 0, 0},
    {-2.63141, 0, 0},
    {-3.31579, 0.522947, 2.03495},
    {-5.36589, -0.0939789, 1.02771},
    {-3.72278, 0, 0},
    {-2.11074, 0, 0},
    {-3.47874, 0.532042, 0.778358},
    {-5.32194, -0.0864, 0.322863},
    {-4.06232, 0, 0},
    {-2.54653, 0, 0},
    {-3.46131, 0.553263, -0.132632},
    {-4.76716, -0.0227368, -0.492632},
    {-3.54073, 0, 0},
    {-2.45634, 0, 0},
    {-3.25137, 0.482779, -1.23613},
    {-4.25937, -0.0227368, -1.12168},
    {-2.83528, 0, 0},
    {-1.79166, 0, 0},
    {3.25624, 7.13148, -0.131575},
    {13.149, -0.052598, -0.125076},
    {27.2903, 0.00282644, -0.0181535},
    {26.6602, 0.000969969, -0.0487599},
    {2.56017, 0.195537, 3.20968},
    {3.78796, 0, 0},
    {2.63141, 0, 0},
    {3.31579, 0.522947, 2.03495},
    {5.36589, -0.0939789, 1.02771},
    {3.72278, 0, 0},
    {2.11074, 0, 0},
    {3.47874, 0.532042, 0.778358},
    {5.32194, -0.0864, 0.322863},
    {4.06232, 0, 0},
    {2.54653, 0, 0},
    {3.46131, 0.553263, -0.132632},
    {4.76716, -0.0227368, -0.492632},
    {3.54073, 0, 0},
    {2.45634, 0, 0},
    {3.25137, 0.482779, -1.23613},
    {4.25937, -0.0227368, -1.12168},
    {2.83528, 0, 0},
    {1.79166, 0, 0}
};

static controller::StandardPoseChannel neuronJointIndexToPoseIndex(NeuronJointIndex i) {
    assert(i >= 0 && i < NeuronJointIndex::Size);
    if (i >= 0 && i < NeuronJointIndex::Size) {
        return neuronJointIndexToPoseIndexMap[i];
    } else {
        return (controller::StandardPoseChannel)0; // not sure what to do here, but don't crash!
    }
}

static const char* controllerJointName(controller::StandardPoseChannel i) {
    switch (i) {
    case controller::HIPS: return "Hips";
    case controller::RIGHT_UP_LEG: return "RightUpLeg";
    case controller::RIGHT_LEG: return "RightLeg";
    case controller::RIGHT_FOOT: return "RightFoot";
    case controller::LEFT_UP_LEG: return "LeftUpLeg";
    case controller::LEFT_LEG: return "LeftLeg";
    case controller::LEFT_FOOT: return "LeftFoot";
    case controller::SPINE: return "Spine";
    case controller::SPINE1: return "Spine1";
    case controller::SPINE2: return "Spine2";
    case controller::SPINE3: return "Spine3";
    case controller::NECK: return "Neck";
    case controller::HEAD: return "Head";
    case controller::RIGHT_SHOULDER: return "RightShoulder";
    case controller::RIGHT_ARM: return "RightArm";
    case controller::RIGHT_FORE_ARM: return "RightForeArm";
    case controller::RIGHT_HAND: return "RightHand";
    case controller::RIGHT_HAND_THUMB1: return "RightHandThumb1";
    case controller::RIGHT_HAND_THUMB2: return "RightHandThumb2";
    case controller::RIGHT_HAND_THUMB3: return "RightHandThumb3";
    case controller::RIGHT_HAND_THUMB4: return "RightHandThumb4";
    case controller::RIGHT_HAND_INDEX1: return "RightHandIndex1";
    case controller::RIGHT_HAND_INDEX2: return "RightHandIndex2";
    case controller::RIGHT_HAND_INDEX3: return "RightHandIndex3";
    case controller::RIGHT_HAND_INDEX4: return "RightHandIndex4";
    case controller::RIGHT_HAND_MIDDLE1: return "RightHandMiddle1";
    case controller::RIGHT_HAND_MIDDLE2: return "RightHandMiddle2";
    case controller::RIGHT_HAND_MIDDLE3: return "RightHandMiddle3";
    case controller::RIGHT_HAND_MIDDLE4: return "RightHandMiddle4";
    case controller::RIGHT_HAND_RING1: return "RightHandRing1";
    case controller::RIGHT_HAND_RING2: return "RightHandRing2";
    case controller::RIGHT_HAND_RING3: return "RightHandRing3";
    case controller::RIGHT_HAND_RING4: return "RightHandRing4";
    case controller::RIGHT_HAND_PINKY1: return "RightHandPinky1";
    case controller::RIGHT_HAND_PINKY2: return "RightHandPinky2";
    case controller::RIGHT_HAND_PINKY3: return "RightHandPinky3";
    case controller::RIGHT_HAND_PINKY4: return "RightHandPinky4";
    case controller::LEFT_SHOULDER: return "LeftShoulder";
    case controller::LEFT_ARM: return "LeftArm";
    case controller::LEFT_FORE_ARM: return "LeftForeArm";
    case controller::LEFT_HAND: return "LeftHand";
    case controller::LEFT_HAND_THUMB1: return "LeftHandThumb1";
    case controller::LEFT_HAND_THUMB2: return "LeftHandThumb2";
    case controller::LEFT_HAND_THUMB3: return "LeftHandThumb3";
    case controller::LEFT_HAND_THUMB4: return "LeftHandThumb4";
    case controller::LEFT_HAND_INDEX1: return "LeftHandIndex1";
    case controller::LEFT_HAND_INDEX2: return "LeftHandIndex2";
    case controller::LEFT_HAND_INDEX3: return "LeftHandIndex3";
    case controller::LEFT_HAND_INDEX4: return "LeftHandIndex4";
    case controller::LEFT_HAND_MIDDLE1: return "LeftHandMiddle1";
    case controller::LEFT_HAND_MIDDLE2: return "LeftHandMiddle2";
    case controller::LEFT_HAND_MIDDLE3: return "LeftHandMiddle3";
    case controller::LEFT_HAND_MIDDLE4: return "LeftHandMiddle4";
    case controller::LEFT_HAND_RING1: return "LeftHandRing1";
    case controller::LEFT_HAND_RING2: return "LeftHandRing2";
    case controller::LEFT_HAND_RING3: return "LeftHandRing3";
    case controller::LEFT_HAND_RING4: return "LeftHandRing4";
    case controller::LEFT_HAND_PINKY1: return "LeftHandPinky1";
    case controller::LEFT_HAND_PINKY2: return "LeftHandPinky2";
    case controller::LEFT_HAND_PINKY3: return "LeftHandPinky3";
    case controller::LEFT_HAND_PINKY4: return "LeftHandPinky4";
    default: return "???";
    }
}

// convert between YXZ neuron euler angles in degrees to quaternion
// this is the default setting in the Axis Neuron server.
static quat eulerToQuat(vec3 euler) {
    // euler.x and euler.y are swaped, WTF.
    glm::vec3 e = glm::vec3(euler.y, euler.x, euler.z) * RADIANS_PER_DEGREE;
    return (glm::angleAxis(e.y, Vectors::UNIT_Y) *
            glm::angleAxis(e.x, Vectors::UNIT_X) *
            glm::angleAxis(e.z, Vectors::UNIT_Z));
}

#ifdef HAVE_NEURON

//
// neuronDataReader SDK callback functions
//

// NOTE: must be thread-safe
void FrameDataReceivedCallback(void* context, SOCKET_REF sender, BvhDataHeaderEx* header, float* data) {

    auto neuronPlugin = reinterpret_cast<NeuronPlugin*>(context);

    // version 1.0
    if (header->DataVersion.Major == 1 && header->DataVersion.Minor == 0) {

        // skip reference joint if present
        if (header->WithReference && header->WithDisp) {
            data += 6;
        } else if (header->WithReference && !header->WithDisp) {
            data += 3;
        }

        if (header->WithDisp) {
            // enter mutex
            std::lock_guard<std::mutex> guard(neuronPlugin->_jointsMutex);

            //
            // Data is 6 floats per joint: 3 position values, 3 rotation euler angles (degrees)
            //

            // resize vector if necessary
            const size_t NUM_FLOATS_PER_JOINT = 6;
            const size_t NUM_JOINTS = header->DataCount / NUM_FLOATS_PER_JOINT;
            if (neuronPlugin->_joints.size() != NUM_JOINTS) {
                neuronPlugin->_joints.resize(NUM_JOINTS, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } });
            }

            assert(sizeof(NeuronPlugin::NeuronJoint) == (NUM_FLOATS_PER_JOINT * sizeof(float)));

            // copy the data
            memcpy(&(neuronPlugin->_joints[0]), data, sizeof(NeuronPlugin::NeuronJoint) * NUM_JOINTS);

        } else {
            qCWarning(inputplugins) << "NeuronPlugin: unsuported binary format, please enable displacements";

            // enter mutex
            std::lock_guard<std::mutex> guard(neuronPlugin->_jointsMutex);

            if (neuronPlugin->_joints.size() != NeuronJointIndex::Size) {
                neuronPlugin->_joints.resize(NeuronJointIndex::Size, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } });
            }

            for (int i = 0; i < NeuronJointIndex::Size; i++) {
                neuronPlugin->_joints[i].euler = glm::vec3();
                neuronPlugin->_joints[i].pos = neuronJointTranslations[i];
            }
        }
    } else {
        static bool ONCE = false;
        if (!ONCE) {
            qCCritical(inputplugins) << "NeuronPlugin: bad frame version number, expected 1.0";
            ONCE = true;
        }
    }
}

// I can't even get the SDK to send me a callback.
// BRCommandFetchAvatarDataFromServer & BRRegisterAutoSyncParmeter [sic] don't seem to work.
// So this is totally untested.
// NOTE: must be thread-safe
static void CommandDataReceivedCallback(void* context, SOCKET_REF sender, CommandPack* pack, void* data) {

    DATA_VER version;
    version._VersionMask = pack->DataVersion;
    if (version.Major == 1 && version.Minor == 0) {
        const char* str = "Unknown";
        switch (pack->CommandId) {
        case Cmd_BoneSize:                // Id can be used to request bone size from server or register avatar name command.
            str = "BoneSize";
            break;
        case Cmd_AvatarName:              // Id can be used to request avatar name from server or register avatar name command.
            str = "AvatarName";
            break;
        case Cmd_FaceDirection:           // Id used to request face direction from server
            str = "FaceDirection";
            break;
        case Cmd_DataFrequency:           // Id can be used to request data frequency from server or register data frequency command.
            str = "DataFrequency";
            break;
        case Cmd_BvhInheritanceTxt:       // Id can be used to request bvh header txt from server or register bvh header txt command.
            str = "BvhInheritanceTxt";
            break;
        case Cmd_AvatarCount:             // Id can be used to request avatar count from server or register avatar count command.
            str = "AvatarCount";
            break;
        case Cmd_CombinationMode:         // Id can be used to request combination mode from server or register combination mode command.
            str = "CombinationMode";
            break;
        case Cmd_RegisterEvent:           // Id can be used to register event.
            str = "RegisterEvent";
            break;
        case Cmd_UnRegisterEvent:         // Id can be used to unregister event.
            str = "UnRegisterEvent";
            break;
        }
        qCDebug(inputplugins) << "NeuronPlugin: command data received CommandID = " << str;
    } else {
        static bool ONCE = false;
        if (!ONCE) {
            qCCritical(inputplugins) << "NeuronPlugin: bad command version number, expected 1.0";
            ONCE = true;
        }
    }
}

// NOTE: must be thread-safe
static void SocketStatusChangedCallback(void* context, SOCKET_REF sender, SocketStatus status, char* message) {
    // just dump to log, later we might want to pop up a connection lost dialog or attempt to reconnect.
    qCDebug(inputplugins) << "NeuronPlugin: socket status = " << message;
}

#endif  // #ifdef HAVE_NEURON

//
// NeuronPlugin
//

bool NeuronPlugin::isSupported() const {
#ifdef HAVE_NEURON
    // Because it's a client/server network architecture, we can't tell
    // if the neuron is actually connected until we connect to the server.
    return true;
#else
    return false;
#endif
}

void NeuronPlugin::activate() {
#ifdef HAVE_NEURON
    InputPlugin::activate();

    // register with userInputMapper
    auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
    userInputMapper->registerDevice(_inputDevice);

    // register c-style callbacks
    BRRegisterFrameDataCallback((void*)this, FrameDataReceivedCallback);
    BRRegisterCommandDataCallback((void*)this, CommandDataReceivedCallback);
    BRRegisterSocketStatusCallback((void*)this, SocketStatusChangedCallback);

    // TODO: Pull these from prefs dialog?
    // localhost is fine for now.
    _serverAddress = "localhost";
    _serverPort = 7001;  // default port for TCP Axis Neuron server.

    _socketRef = BRConnectTo((char*)_serverAddress.c_str(), _serverPort);
    if (!_socketRef) {
        // error
        qCCritical(inputplugins) << "NeuronPlugin: error connecting to " << _serverAddress.c_str() << ":" << _serverPort << ", error = " << BRGetLastErrorMessage();
    } else {
        qCDebug(inputplugins) << "NeuronPlugin: success connecting to " << _serverAddress.c_str() << ":" << _serverPort;

        BRRegisterAutoSyncParmeter(_socketRef, Cmd_CombinationMode);
    }
#endif
}

void NeuronPlugin::deactivate() {
#ifdef HAVE_NEURON
    // unregister from userInputMapper
    if (_inputDevice->_deviceID != controller::Input::INVALID_DEVICE) {
        auto userInputMapper = DependencyManager::get<controller::UserInputMapper>();
        userInputMapper->removeDevice(_inputDevice->_deviceID);
    }

    if (_socketRef) {
        BRUnregisterAutoSyncParmeter(_socketRef, Cmd_CombinationMode);
        BRCloseSocket(_socketRef);
    }

    InputPlugin::deactivate();
#endif
}

void NeuronPlugin::pluginUpdate(float deltaTime, const controller::InputCalibrationData& inputCalibrationData, bool jointsCaptured) {
    std::vector<NeuronJoint> joints;
    {
        // lock and copy
        std::lock_guard<std::mutex> guard(_jointsMutex);
        joints = _joints;
    }
    _inputDevice->update(deltaTime, inputCalibrationData, joints, _prevJoints);
    _prevJoints = joints;
}

void NeuronPlugin::saveSettings() const {
    InputPlugin::saveSettings();
}

void NeuronPlugin::loadSettings() {
    InputPlugin::loadSettings();
}

//
// InputDevice
//

controller::Input::NamedVector NeuronPlugin::InputDevice::getAvailableInputs() const {
    static controller::Input::NamedVector availableInputs;
    if (availableInputs.size() == 0) {
        for (int i = 0; i < controller::NUM_STANDARD_POSES; i++) {
            auto channel = static_cast<controller::StandardPoseChannel>(i);
            availableInputs.push_back(makePair(channel, controllerJointName(channel)));
        }
    };
    return availableInputs;
}

QString NeuronPlugin::InputDevice::getDefaultMappingConfig() const {
    static const QString MAPPING_JSON = PathUtils::resourcesPath() + "/controllers/neuron.json";
    return MAPPING_JSON;
}

void NeuronPlugin::InputDevice::update(float deltaTime, const controller::InputCalibrationData& inputCalibrationData, const std::vector<NeuronPlugin::NeuronJoint>& joints, const std::vector<NeuronPlugin::NeuronJoint>& prevJoints) {
    for (size_t i = 0; i < joints.size(); i++) {
        glm::vec3 linearVel, angularVel;
        glm::vec3 pos = joints[i].pos;
        glm::quat rot = eulerToQuat(joints[i].euler);
        if (i < prevJoints.size()) {
            linearVel = (pos - (prevJoints[i].pos * METERS_PER_CENTIMETER)) / deltaTime;  // m/s
            // quat log imaginary part points along the axis of rotation, with length of one half the angle of rotation.
            glm::quat d = glm::log(rot * glm::inverse(eulerToQuat(prevJoints[i].euler)));
            angularVel = glm::vec3(d.x, d.y, d.z) / (0.5f * deltaTime); // radians/s
        }
        int poseIndex = neuronJointIndexToPoseIndex((NeuronJointIndex)i);
        _poseStateMap[poseIndex] = controller::Pose(pos, rot, linearVel, angularVel);
    }

    _poseStateMap[controller::RIGHT_HAND_THUMB1] = controller::Pose(rightHandThumb1DefaultAbsTranslation, glm::quat(), glm::vec3(), glm::vec3());
    _poseStateMap[controller::LEFT_HAND_THUMB1] = controller::Pose(leftHandThumb1DefaultAbsTranslation, glm::quat(), glm::vec3(), glm::vec3());
}
