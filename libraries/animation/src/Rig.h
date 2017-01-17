//
//  Rig.h
//  libraries/animation/src/
//
//  Produces animation data and hip placement for the current timestamp.
//
//  Created by Howard Stearns, Seth Alves, Anthony Thibault, Andrew Meadows on 7/15/15.
//  Copyright (c) 2015 High Fidelity, Inc. All rights reserved.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef __hifi__Rig__
#define __hifi__Rig__

#include <QObject>
#include <QMutex>
#include <QScriptValue>
#include <vector>
#include <JointData.h>
#include <QReadWriteLock>

#include "AnimNode.h"
#include "AnimNodeLoader.h"
#include "SimpleMovingAverage.h"

class Rig;
typedef std::shared_ptr<Rig> RigPointer;

// Rig instances are reentrant.
// However only specific methods thread-safe.  Noted below.

class Rig : public QObject, public std::enable_shared_from_this<Rig> {
    Q_OBJECT
public:
    struct StateHandler {
        AnimVariantMap results;
        QStringList propertyNames;
        QScriptValue function;
        bool useNames;
    };

    struct HeadParameters {
        glm::quat worldHeadOrientation = glm::quat();  // world space (-z forward)
        glm::quat rigHeadOrientation = glm::quat();  // rig space (-z forward)
        glm::vec3 rigHeadPosition = glm::vec3();     // rig space
        bool isInHMD = false;
        int neckJointIndex = -1;
        bool isTalking = false;
    };

    struct EyeParameters {
        glm::quat worldHeadOrientation = glm::quat();
        glm::vec3 eyeLookAt = glm::vec3();  // world space
        glm::vec3 eyeSaccade = glm::vec3(); // world space
        glm::vec3 modelTranslation = glm::vec3();
        glm::quat modelRotation = glm::quat();
        int leftEyeJointIndex = -1;
        int rightEyeJointIndex = -1;
    };

    struct HandParameters {
        bool isLeftEnabled;
        bool isRightEnabled;
        float bodyCapsuleRadius;
        float bodyCapsuleHalfHeight;
        glm::vec3 bodyCapsuleLocalOffset;
        glm::vec3 leftPosition = glm::vec3();     // rig space
        glm::quat leftOrientation = glm::quat();  // rig space (z forward)
        glm::vec3 rightPosition = glm::vec3();    // rig space
        glm::quat rightOrientation = glm::quat(); // rig space (z forward)
    };

    enum class CharacterControllerState {
        Ground = 0,
        Takeoff,
        InAir,
        Hover
    };

    Rig() {}
    virtual ~Rig() {}

    void destroyAnimGraph();

    void overrideAnimation(const QString& url, float fps, bool loop, float firstFrame, float lastFrame);
    void restoreAnimation();
    QStringList getAnimationRoles() const;
    void overrideRoleAnimation(const QString& role, const QString& url, float fps, bool loop, float firstFrame, float lastFrame);
    void restoreRoleAnimation(const QString& role);

    void initJointStates(const FBXGeometry& geometry, const glm::mat4& modelOffset);
    void reset(const FBXGeometry& geometry);
    bool jointStatesEmpty();
    int getJointStateCount() const;
    int indexOfJoint(const QString& jointName) const;
    QString nameOfJoint(int jointIndex) const;

    void setModelOffset(const glm::mat4& modelOffsetMat);

    void clearJointState(int index);
    void clearJointStates();
    void clearJointAnimationPriority(int index);

    void clearIKJointLimitHistory();

    int getJointParentIndex(int childIndex) const;

    // geometry space
    void setJointState(int index, bool valid, const glm::quat& rotation, const glm::vec3& translation, float priority);

    // geometry space
    void setJointTranslation(int index, bool valid, const glm::vec3& translation, float priority);
    void setJointRotation(int index, bool valid, const glm::quat& rotation, float priority);

    // legacy
    void restoreJointRotation(int index, float fraction, float priority);
    void restoreJointTranslation(int index, float fraction, float priority);

    // if translation and rotation is identity, position will be in rig space
    bool getJointPositionInWorldFrame(int jointIndex, glm::vec3& position,
                                      glm::vec3 translation, glm::quat rotation) const;

    // rig space
    bool getJointPosition(int jointIndex, glm::vec3& position) const;

    // if rotation is identity, result will be in rig space
    bool getJointRotationInWorldFrame(int jointIndex, glm::quat& result, const glm::quat& rotation) const;

    // geometry space (thread-safe)
    bool getJointRotation(int jointIndex, glm::quat& rotation) const;
    bool getJointTranslation(int jointIndex, glm::vec3& translation) const;

    // rig space (thread-safe)
    bool getAbsoluteJointRotationInRigFrame(int jointIndex, glm::quat& rotation) const;
    bool getAbsoluteJointTranslationInRigFrame(int jointIndex, glm::vec3& translation) const;
    bool getAbsoluteJointPoseInRigFrame(int jointIndex, AnimPose& returnPose) const;

    // legacy
    bool getJointCombinedRotation(int jointIndex, glm::quat& result, const glm::quat& rotation) const;

    // rig space
    glm::mat4 getJointTransform(int jointIndex) const;

    // Start or stop animations as needed.
    void computeMotionAnimationState(float deltaTime, const glm::vec3& worldPosition, const glm::vec3& worldVelocity, const glm::quat& worldRotation, CharacterControllerState ccState);

    // Regardless of who started the animations or how many, update the joints.
    void updateAnimations(float deltaTime, glm::mat4 rootTransform);

    // legacy
    void inverseKinematics(int endIndex, glm::vec3 targetPosition, const glm::quat& targetRotation, float priority,
                           const QVector<int>& freeLineage, glm::mat4 rootTransform);

    // legacy
    bool restoreJointPosition(int jointIndex, float fraction, float priority, const QVector<int>& freeLineage);

    // legacy
    float getLimbLength(int jointIndex, const QVector<int>& freeLineage,
                        const glm::vec3 scale, const QVector<FBXJoint>& fbxJoints) const;

    // legacy
    glm::quat setJointRotationInBindFrame(int jointIndex, const glm::quat& rotation, float priority);

    // legacy
    glm::vec3 getJointDefaultTranslationInConstrainedFrame(int jointIndex);

    // legacy
    glm::quat setJointRotationInConstrainedFrame(int jointIndex, glm::quat targetRotation,
                                                 float priority, float mix = 1.0f);

    // legacy
    bool getJointRotationInConstrainedFrame(int jointIndex, glm::quat& rotOut) const;

    // legacy
    glm::quat getJointDefaultRotationInParentFrame(int jointIndex);

    // legacy
    void clearJointStatePriorities();

    void updateFromHeadParameters(const HeadParameters& params, float dt);
    void updateFromEyeParameters(const EyeParameters& params);
    void updateFromHandParameters(const HandParameters& params, float dt);

    void initAnimGraph(const QUrl& url);

    AnimNode::ConstPointer getAnimNode() const { return _animNode; }
    AnimSkeleton::ConstPointer getAnimSkeleton() const { return _animSkeleton; }
    QScriptValue addAnimationStateHandler(QScriptValue handler, QScriptValue propertiesList);
    void removeAnimationStateHandler(QScriptValue handler);
    void animationStateHandlerResult(int identifier, QScriptValue result);

    // rig space
    bool getModelRegistrationPoint(glm::vec3& modelRegistrationPointOut) const;

    // rig space
    AnimPose getAbsoluteDefaultPose(int index) const;

    // rig space
    const AnimPoseVec& getAbsoluteDefaultPoses() const;

    // geometry space
    bool getRelativeDefaultJointRotation(int index, glm::quat& rotationOut) const;
    bool getRelativeDefaultJointTranslation(int index, glm::vec3& translationOut) const;

    void copyJointsIntoJointData(QVector<JointData>& jointDataVec) const;
    void copyJointsFromJointData(const QVector<JointData>& jointDataVec);

    void computeAvatarBoundingCapsule(const FBXGeometry& geometry, float& radiusOut, float& heightOut, glm::vec3& offsetOut) const;

    void setEnableInverseKinematics(bool enable);

    const glm::mat4& getGeometryToRigTransform() const { return _geometryToRigTransform; }

signals:
    void onLoadComplete();

protected:
    bool isIndexValid(int index) const { return _animSkeleton && index >= 0 && index < _animSkeleton->getNumJoints(); }
    void updateAnimationStateHandlers();
    void applyOverridePoses();
    void buildAbsoluteRigPoses(const AnimPoseVec& relativePoses, AnimPoseVec& absolutePosesOut);

    void updateNeckJoint(int index, const HeadParameters& params);
    void computeHeadNeckAnimVars(const AnimPose& hmdPose, glm::vec3& headPositionOut, glm::quat& headOrientationOut,
                                 glm::vec3& neckPositionOut, glm::quat& neckOrientationOut) const;
    void updateEyeJoint(int index, const glm::vec3& modelTranslation, const glm::quat& modelRotation, const glm::quat& worldHeadOrientation, const glm::vec3& lookAt, const glm::vec3& saccade);
    void calcAnimAlpha(float speed, const std::vector<float>& referenceSpeeds, float* alphaOut) const;

    AnimPose _modelOffset;  // model to rig space
    AnimPose _geometryOffset; // geometry to model space (includes unit offset & fst offsets)
    AnimPose _invGeometryOffset;

    struct PoseSet {
        AnimPoseVec _relativePoses; // geometry space relative to parent.
        AnimPoseVec _absolutePoses; // rig space, not relative to parent.
        AnimPoseVec _overridePoses; // geometry space relative to parent.
        std::vector<bool> _overrideFlags;
    };

    // Only accessed by the main thread
    PoseSet _internalPoseSet;

    // Copy of the _poseSet for external threads.
    PoseSet _externalPoseSet;
    mutable QReadWriteLock _externalPoseSetLock;

    AnimPoseVec _absoluteDefaultPoses; // rig space, not relative to parent.

    glm::mat4 _geometryToRigTransform;
    glm::mat4 _rigToGeometryTransform;

    int _rootJointIndex { -1 };

    int _leftHandJointIndex { -1 };
    int _leftElbowJointIndex { -1 };
    int _leftShoulderJointIndex { -1 };

    int _rightHandJointIndex { -1 };
    int _rightElbowJointIndex { -1 };
    int _rightShoulderJointIndex { -1 };

    glm::vec3 _lastFront;
    glm::vec3 _lastPosition;
    glm::vec3 _lastVelocity;

    QUrl _animGraphURL;
    std::shared_ptr<AnimNode> _animNode;
    std::shared_ptr<AnimSkeleton> _animSkeleton;
    std::unique_ptr<AnimNodeLoader> _animLoader;
    AnimVariantMap _animVars;
    enum class RigRole {
        Idle = 0,
        Turn,
        Move,
        Hover,
        Takeoff,
        InAir
    };
    RigRole _state { RigRole::Idle };
    RigRole _desiredState { RigRole::Idle };
    float _desiredStateAge { 0.0f };

    struct UserAnimState {
        enum ClipNodeEnum {
            None = 0,
            A,
            B
        };

        UserAnimState() : clipNodeEnum(UserAnimState::None) {}
        UserAnimState(ClipNodeEnum clipNodeEnumIn, const QString& urlIn, float fpsIn, bool loopIn, float firstFrameIn, float lastFrameIn) :
            clipNodeEnum(clipNodeEnumIn), url(urlIn), fps(fpsIn), loop(loopIn), firstFrame(firstFrameIn), lastFrame(lastFrameIn) {}

        ClipNodeEnum clipNodeEnum;
        QString url;
        float fps;
        bool loop;
        float firstFrame;
        float lastFrame;
    };

    UserAnimState _userAnimState;

    float _leftHandOverlayAlpha { 0.0f };
    float _rightHandOverlayAlpha { 0.0f };

    SimpleMovingAverage _averageForwardSpeed { 10 };
    SimpleMovingAverage _averageLateralSpeed { 10 };

    std::map<QString, AnimNode::Pointer> _origRoleAnimations;

    int32_t _numOverrides { 0 };
    bool _lastEnableInverseKinematics { true };
    bool _enableInverseKinematics { true };

    mutable uint32_t _jointNameWarningCount { 0 };

private:
    QMap<int, StateHandler> _stateHandlers;
    int _nextStateHandlerId { 0 };
    QMutex _stateMutex;
};

#endif /* defined(__hifi__Rig__) */
