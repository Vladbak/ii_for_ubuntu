//
//  MyAvatar.h
//  interface/src/avatar
//
//  Created by Mark Peng on 8/16/13.
//  Copyright 2012 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_MyAvatar_h
#define hifi_MyAvatar_h

#include <glm/glm.hpp>

#include <SettingHandle.h>
#include <Rig.h>

#include <controllers/Pose.h>

#include "Avatar.h"
#include "AtRestDetector.h"
#include "MyCharacterController.h"


class ModelItemID;

enum DriveKeys {
    TRANSLATE_X = 0,
    TRANSLATE_Y,
    TRANSLATE_Z,
    YAW,
    STEP_TRANSLATE_X,
    STEP_TRANSLATE_Y,
    STEP_TRANSLATE_Z,
    STEP_YAW,
    PITCH,
    ZOOM,
    MAX_DRIVE_KEYS
};

enum eyeContactTarget {
    LEFT_EYE,
    RIGHT_EYE,
    MOUTH
};

enum AudioListenerMode {
    FROM_HEAD = 0,
    FROM_CAMERA,
    CUSTOM
};
Q_DECLARE_METATYPE(AudioListenerMode);

class MyAvatar : public Avatar {
    Q_OBJECT
    Q_PROPERTY(bool shouldRenderLocally READ getShouldRenderLocally WRITE setShouldRenderLocally)
    Q_PROPERTY(glm::vec3 motorVelocity READ getScriptedMotorVelocity WRITE setScriptedMotorVelocity)
    Q_PROPERTY(float motorTimescale READ getScriptedMotorTimescale WRITE setScriptedMotorTimescale)
    Q_PROPERTY(QString motorReferenceFrame READ getScriptedMotorFrame WRITE setScriptedMotorFrame)
    Q_PROPERTY(QString collisionSoundURL READ getCollisionSoundURL WRITE setCollisionSoundURL)
    Q_PROPERTY(AudioListenerMode audioListenerMode READ getAudioListenerMode WRITE setAudioListenerMode)
    Q_PROPERTY(glm::vec3 customListenPosition READ getCustomListenPosition WRITE setCustomListenPosition)
    Q_PROPERTY(glm::quat customListenOrientation READ getCustomListenOrientation WRITE setCustomListenOrientation)
    Q_PROPERTY(AudioListenerMode FROM_HEAD READ getAudioListenerModeHead)
    Q_PROPERTY(AudioListenerMode FROM_CAMERA READ getAudioListenerModeCamera)
    Q_PROPERTY(AudioListenerMode CUSTOM READ getAudioListenerModeCustom)
    //TODO: make gravity feature work Q_PROPERTY(glm::vec3 gravity READ getGravity WRITE setGravity)


    Q_PROPERTY(glm::vec3 leftHandPosition READ getLeftHandPosition)
    Q_PROPERTY(glm::vec3 rightHandPosition READ getRightHandPosition)
    Q_PROPERTY(glm::vec3 leftHandTipPosition READ getLeftHandTipPosition)
    Q_PROPERTY(glm::vec3 rightHandTipPosition READ getRightHandTipPosition)

    Q_PROPERTY(controller::Pose leftHandPose READ getLeftHandPose)
    Q_PROPERTY(controller::Pose rightHandPose READ getRightHandPose)
    Q_PROPERTY(controller::Pose leftHandTipPose READ getLeftHandTipPose)
    Q_PROPERTY(controller::Pose rightHandTipPose READ getRightHandTipPose)
    Q_PROPERTY(float energy READ getEnergy WRITE setEnergy)

public:
    MyAvatar(RigPointer rig);
    ~MyAvatar();

    virtual void simulateAttachments(float deltaTime) override;

    AudioListenerMode getAudioListenerModeHead() const { return FROM_HEAD; }
    AudioListenerMode getAudioListenerModeCamera() const { return FROM_CAMERA; }
    AudioListenerMode getAudioListenerModeCustom() const { return CUSTOM; }

    void reset(bool andReload = false);
    void update(float deltaTime);
    void preRender(RenderArgs* renderArgs);

    const glm::mat4& getHMDSensorMatrix() const { return _hmdSensorMatrix; }
    const glm::vec3& getHMDSensorPosition() const { return _hmdSensorPosition; }
    const glm::quat& getHMDSensorOrientation() const { return _hmdSensorOrientation; }
    const glm::vec2& getHMDSensorFacingMovingAverage() const { return _hmdSensorFacingMovingAverage; }
    glm::mat4 getSensorToWorldMatrix() const;


    // Pass a recent sample of the HMD to the avatar.
    // This can also update the avatar's position to follow the HMD
    // as it moves through the world.
    void updateFromHMDSensorMatrix(const glm::mat4& hmdSensorMatrix);

    // best called at end of main loop, just before rendering.
    // update sensor to world matrix from current body position and hmd sensor.
    // This is so the correct camera can be used for rendering.
    void updateSensorToWorldMatrix();

    void setRealWorldFieldOfView(float realWorldFov) { _realWorldFieldOfView.set(realWorldFov); }

    void setLeanScale(float scale) { _leanScale = scale; }
    float getLeanScale() const { return _leanScale; }

    Q_INVOKABLE glm::vec3 getDefaultEyePosition() const;

    float getRealWorldFieldOfView() { return _realWorldFieldOfView.get(); }

    // Interrupt the current animation with a custom animation.
    Q_INVOKABLE void overrideAnimation(const QString& url, float fps, bool loop, float firstFrame, float lastFrame);

    // Stop the animation that was started with overrideAnimation and go back to the standard animation.
    Q_INVOKABLE void restoreAnimation();

    // Returns a list of all clips that are available
    Q_INVOKABLE QStringList getAnimationRoles();

    // Replace an existing standard role animation with a custom one.
    Q_INVOKABLE void overrideRoleAnimation(const QString& role, const QString& url, float fps, bool loop, float firstFrame, float lastFrame);

    // remove an animation role override and return to the standard animation.
    Q_INVOKABLE void restoreRoleAnimation(const QString& role);

    // prefetch animation
    Q_INVOKABLE void prefetchAnimation(const QString& url);

    // Adds handler(animStateDictionaryIn) => animStateDictionaryOut, which will be invoked just before each animGraph state update.
    // The handler will be called with an animStateDictionaryIn that has all those properties specified by the (possibly empty)
    // propertiesList argument. However for debugging, if the properties argument is null, all internal animGraph state is provided.
    // The animStateDictionaryOut can be a different object than animStateDictionaryIn. Any properties set in animStateDictionaryOut
    // will override those of the internal animation machinery.
    // The animStateDictionaryIn may be shared among multiple handlers, and thus may contain additional properties specified when
    // adding one of the other handlers. While any handler may change a value in animStateDictionaryIn (or supply different values in animStateDictionaryOut)
    // a handler must not remove properties from animStateDictionaryIn, nor change property values that it does not intend to change.
    // It is not specified in what order multiple handlers are called.
    Q_INVOKABLE QScriptValue addAnimationStateHandler(QScriptValue handler, QScriptValue propertiesList) { return _rig->addAnimationStateHandler(handler, propertiesList); }
    // Removes a handler previously added by addAnimationStateHandler.
    Q_INVOKABLE void removeAnimationStateHandler(QScriptValue handler) { _rig->removeAnimationStateHandler(handler); }

    Q_INVOKABLE bool getSnapTurn() const { return _useSnapTurn; }
    Q_INVOKABLE void setSnapTurn(bool on) { _useSnapTurn = on; }

    // get/set avatar data
    void saveData();
    void loadData();

    void saveAttachmentData(const AttachmentData& attachment) const;
    AttachmentData loadAttachmentData(const QUrl& modelURL, const QString& jointName = QString()) const;

    //  Set what driving keys are being pressed to control thrust levels
    void clearDriveKeys();
    void setDriveKeys(int key, float val) { _driveKeys[key] = val; };
    void relayDriveKeysToCharacterController();

    eyeContactTarget getEyeContactTarget();

    Q_INVOKABLE glm::vec3 getTrackedHeadPosition() const { return _trackedHeadPosition; }
    Q_INVOKABLE glm::vec3 getHeadPosition() const { return getHead()->getPosition(); }
    Q_INVOKABLE float getHeadFinalYaw() const { return getHead()->getFinalYaw(); }
    Q_INVOKABLE float getHeadFinalRoll() const { return getHead()->getFinalRoll(); }
    Q_INVOKABLE float getHeadFinalPitch() const { return getHead()->getFinalPitch(); }
    Q_INVOKABLE float getHeadDeltaPitch() const { return getHead()->getDeltaPitch(); }
    Q_INVOKABLE int getFaceBlendCoefNum() const { return getHead()->getFaceModel().getBlendshapeCoefficientsNum(); }
    Q_INVOKABLE float getFaceBlendCoef(int index) const { return getHead()->getFaceModel().getBlendshapeCoefficient(index); }

    Q_INVOKABLE glm::vec3 getEyePosition() const { return getHead()->getEyePosition(); }

    Q_INVOKABLE glm::vec3 getTargetAvatarPosition() const { return _targetAvatarPosition; }

    Q_INVOKABLE glm::vec3 getLeftHandPosition() const;
    Q_INVOKABLE glm::vec3 getRightHandPosition() const;
    Q_INVOKABLE glm::vec3 getLeftHandTipPosition() const;
    Q_INVOKABLE glm::vec3 getRightHandTipPosition() const;

    Q_INVOKABLE controller::Pose getLeftHandPose() const;
    Q_INVOKABLE controller::Pose getRightHandPose() const;
    Q_INVOKABLE controller::Pose getLeftHandTipPose() const;
    Q_INVOKABLE controller::Pose getRightHandTipPose() const;

    AvatarWeakPointer getLookAtTargetAvatar() const { return _lookAtTargetAvatar; }
    void updateLookAtTargetAvatar();
    void clearLookAtTargetAvatar();

    virtual void setJointRotations(QVector<glm::quat> jointRotations) override;
    virtual void setJointData(int index, const glm::quat& rotation, const glm::vec3& translation) override;
    virtual void setJointRotation(int index, const glm::quat& rotation) override;
    virtual void setJointTranslation(int index, const glm::vec3& translation) override;
    virtual void clearJointData(int index) override;
    virtual void clearJointsData() override;

    Q_INVOKABLE void useFullAvatarURL(const QUrl& fullAvatarURL, const QString& modelName = QString());
    Q_INVOKABLE const QUrl& getFullAvatarURLFromPreferences() const { return _fullAvatarURLFromPreferences; }
    Q_INVOKABLE const QString& getFullAvatarModelName() const { return _fullAvatarModelName; }
    void resetFullAvatarURL();


    virtual void setAttachmentData(const QVector<AttachmentData>& attachmentData) override;

    MyCharacterController* getCharacterController() { return &_characterController; }
    const MyCharacterController* getCharacterController() const { return &_characterController; }

    void prepareForPhysicsSimulation();
    void harvestResultsFromPhysicsSimulation(float deltaTime);

    const QString& getCollisionSoundURL() { return _collisionSoundURL; }
    void setCollisionSoundURL(const QString& url);

    void clearScriptableSettings();

    float getBoomLength() const { return _boomLength; }
    void setBoomLength(float boomLength) { _boomLength = boomLength; }

    float getPitchSpeed() const { return _pitchSpeed; }
    void setPitchSpeed(float speed) { _pitchSpeed = speed; }

    float getYawSpeed() const { return _yawSpeed; }
    void setYawSpeed(float speed) { _yawSpeed = speed; }

    static const float ZOOM_MIN;
    static const float ZOOM_MAX;
    static const float ZOOM_DEFAULT;

    void destroyAnimGraph();

    AudioListenerMode getAudioListenerMode() { return _audioListenerMode; }
    void setAudioListenerMode(AudioListenerMode audioListenerMode);
    glm::vec3 getCustomListenPosition() { return _customListenPosition; }
    void setCustomListenPosition(glm::vec3 customListenPosition) { _customListenPosition = customListenPosition; }
    glm::quat getCustomListenOrientation() { return _customListenOrientation; }
    void setCustomListenOrientation(glm::quat customListenOrientation) { _customListenOrientation = customListenOrientation; }

    virtual void rebuildCollisionShape() override;

public slots:
    void increaseSize();
    void decreaseSize();
    void resetSize();

    void goToLocation(const glm::vec3& newPosition,
                      bool hasOrientation = false, const glm::quat& newOrientation = glm::quat(),
                      bool shouldFaceLocation = false);

    //  Set/Get update the thrust that will move the avatar around
    void addThrust(glm::vec3 newThrust) { _thrust += newThrust; };
    glm::vec3 getThrust() { return _thrust; };
    void setThrust(glm::vec3 newThrust) { _thrust = newThrust; }

    Q_INVOKABLE void updateMotionBehaviorFromMenu();

    Q_INVOKABLE QUrl getAnimGraphUrl() const { return _animGraphUrl; }

    void setEnableDebugDrawDefaultPose(bool isEnabled);
    void setEnableDebugDrawAnimPose(bool isEnabled);
    void setEnableDebugDrawPosition(bool isEnabled);
    bool getEnableMeshVisible() const { return _skeletonModel.isVisible(); }
    void setEnableMeshVisible(bool isEnabled);
    void setUseAnimPreAndPostRotations(bool isEnabled);
    void setEnableInverseKinematics(bool isEnabled);
    Q_INVOKABLE void setAnimGraphUrl(const QUrl& url);

    glm::vec3 getPositionForAudio();
    glm::quat getOrientationForAudio();

signals:
    void audioListenerModeChanged();
    void transformChanged();
    void newCollisionSoundURL(const QUrl& url);
    void collisionWithEntity(const Collision& collision);
    void energyChanged(float newEnergy);
    void positionGoneTo();


private:

    glm::vec3 getWorldBodyPosition() const;
    glm::quat getWorldBodyOrientation() const;
    QByteArray toByteArray(bool cullSmallChanges, bool sendAll) override;
    void simulate(float deltaTime);
    void updateFromTrackers(float deltaTime);
    virtual void render(RenderArgs* renderArgs, const glm::vec3& cameraPositio) override;
    virtual void renderBody(RenderArgs* renderArgs, ViewFrustum* renderFrustum, float glowLevel = 0.0f) override;
    virtual bool shouldRenderHead(const RenderArgs* renderArgs) const override;
    void setShouldRenderLocally(bool shouldRender) { _shouldRender = shouldRender; setEnableMeshVisible(shouldRender); }
    bool getShouldRenderLocally() const { return _shouldRender; }
    bool getDriveKeys(int key) { return _driveKeys[key] != 0.0f; };
    bool isMyAvatar() const override { return true; }
    virtual int parseDataFromBuffer(const QByteArray& buffer) override;
    virtual glm::vec3 getSkeletonPosition() const override;

    glm::vec3 getScriptedMotorVelocity() const { return _scriptedMotorVelocity; }
    float getScriptedMotorTimescale() const { return _scriptedMotorTimescale; }
    QString getScriptedMotorFrame() const;
    void setScriptedMotorVelocity(const glm::vec3& velocity);
    void setScriptedMotorTimescale(float timescale);
    void setScriptedMotorFrame(QString frame);
    virtual void attach(const QString& modelURL, const QString& jointName = QString(),
                        const glm::vec3& translation = glm::vec3(), const glm::quat& rotation = glm::quat(),
                        float scale = 1.0f, bool isSoft = false,
                        bool allowDuplicates = false, bool useSaved = true) override;

    bool cameraInsideHead() const;

    // These are made private for MyAvatar so that you will use the "use" methods instead
    virtual void setFaceModelURL(const QUrl& faceModelURL) override;
    virtual void setSkeletonModelURL(const QUrl& skeletonModelURL) override;

    void setVisibleInSceneIfReady(Model* model, render::ScenePointer scene, bool visiblity);

    // derive avatar body position and orientation from the current HMD Sensor location.
    // results are in HMD frame
    glm::mat4 deriveBodyFromHMDSensor() const;

    virtual void updatePalms() override {}
    void lateUpdatePalms();


    float _driveKeys[MAX_DRIVE_KEYS];
    bool _wasPushing;
    bool _isPushing;
    bool _isBraking;

    float _boomLength;
    float _yawSpeed; // degrees/sec
    float _pitchSpeed; // degrees/sec

    glm::vec3 _thrust;  // impulse accumulator for outside sources

    glm::vec3 _keyboardMotorVelocity; // target local-frame velocity of avatar (keyboard)
    float _keyboardMotorTimescale; // timescale for avatar to achieve its target velocity
    glm::vec3 _scriptedMotorVelocity; // target local-frame velocity of avatar (script)
    float _scriptedMotorTimescale; // timescale for avatar to achieve its target velocity
    int _scriptedMotorFrame;
    quint32 _motionBehaviors;
    QString _collisionSoundURL;

    MyCharacterController _characterController;

    AvatarWeakPointer _lookAtTargetAvatar;
    glm::vec3 _targetAvatarPosition;
    bool _shouldRender;
    float _oculusYawOffset;

    eyeContactTarget _eyeContactTarget;

    glm::vec3 _trackedHeadPosition;

    Setting::Handle<float> _realWorldFieldOfView;

    // private methods
    void updateOrientation(float deltaTime);
    glm::vec3 applyKeyboardMotor(float deltaTime, const glm::vec3& velocity, bool isHovering);
    glm::vec3 applyScriptedMotor(float deltaTime, const glm::vec3& velocity);
    void updatePosition(float deltaTime);
    void updateCollisionSound(const glm::vec3& penetration, float deltaTime, float frequency);
    void initHeadBones();
    void initAnimGraph();

    // Avatar Preferences
    QUrl _fullAvatarURLFromPreferences;
    QString _fullAvatarModelName;
    QUrl _animGraphUrl {""};
    bool _useSnapTurn { true };

    // cache of the current HMD sensor position and orientation
    // in sensor space.
    glm::mat4 _hmdSensorMatrix;
    glm::quat _hmdSensorOrientation;
    glm::vec3 _hmdSensorPosition;
    glm::vec2 _hmdSensorFacing;  // facing vector in xz plane
    glm::vec2 _hmdSensorFacingMovingAverage { 0, 0 };   // facing vector in xz plane

    // cache of the current body position and orientation of the avatar's body,
    // in sensor space.
    glm::mat4 _bodySensorMatrix;

    // used to transform any sensor into world space, including the _hmdSensorMat, or hand controllers.
    glm::mat4 _sensorToWorldMatrix;

    struct FollowHelper {
        glm::mat4 _desiredBodyMatrix;
        float _timeRemaining { 0.0f };

        void deactivate();
        void activate();
        bool isActive() const;
        bool shouldActivate(const MyAvatar& myAvatar, const glm::mat4& desiredBodyMatrix, const glm::mat4& currentBodyMatrix) const;
        void prePhysicsUpdate(MyAvatar& myAvatar, const glm::mat4& bodySensorMatrix, const glm::mat4& currentBodyMatrix);
        glm::mat4 postPhysicsUpdate(const MyAvatar& myAvatar, const glm::mat4& currentBodyMatrix);
    };
    FollowHelper _follow;

    bool _goToPending;
    glm::vec3 _goToPosition;
    glm::quat _goToOrientation;

    std::unordered_set<int> _headBoneSet;
    RigPointer _rig;
    bool _prevShouldDrawHead;

    bool _enableDebugDrawDefaultPose { false };
    bool _enableDebugDrawAnimPose { false };

    AudioListenerMode _audioListenerMode;
    glm::vec3 _customListenPosition;
    glm::quat _customListenOrientation;

    AtRestDetector _hmdAtRestDetector;
    bool _lastIsMoving { false };
    bool _hoverReferenceCameraFacingIsCaptured { false };
    glm::vec3 _hoverReferenceCameraFacing { 0.0f, 0.0f, -1.0f }; // hmd sensor space

    float AVATAR_MOVEMENT_ENERGY_CONSTANT { 0.001f };
    float AUDIO_ENERGY_CONSTANT { 0.000001f };
    float MAX_AVATAR_MOVEMENT_PER_FRAME { 30.0f };
    float currentEnergy { 0.0f };
    float energyChargeRate { 0.003f };
    glm::vec3 priorVelocity;
    glm::vec3 lastPosition;
    float getAudioEnergy();
    float getAccelerationEnergy();
    float getEnergy();
    void setEnergy(float value);
    bool didTeleport();
};

QScriptValue audioListenModeToScriptValue(QScriptEngine* engine, const AudioListenerMode& audioListenerMode);
void audioListenModeFromScriptValue(const QScriptValue& object, AudioListenerMode& audioListenerMode);

#endif // hifi_MyAvatar_h
