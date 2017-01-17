//
//  AvatarManager.h
//  interface/src/avatar
//
//  Created by Stephen Birarda on 1/23/2014.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#ifndef hifi_AvatarManager_h
#define hifi_AvatarManager_h

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QSharedPointer>

#include <AvatarHashMap.h>
#include <PhysicsEngine.h>
#include <PIDController.h>
#include <SimpleMovingAverage.h>
#include <shared/RateCounter.h>

#include "Avatar.h"
#include "AvatarMotionState.h"

class MyAvatar;
class AudioInjector;

class AvatarManager : public AvatarHashMap {
    Q_OBJECT
    SINGLETON_DEPENDENCY

public:
    /// Registers the script types associated with the avatar manager.
    static void registerMetaTypes(QScriptEngine* engine);

    virtual ~AvatarManager();

    void init();

    std::shared_ptr<MyAvatar> getMyAvatar() { return _myAvatar; }
    AvatarSharedPointer getAvatarBySessionID(const QUuid& sessionID) override;

    void updateMyAvatar(float deltaTime);
    void updateOtherAvatars(float deltaTime);

    void postUpdate(float deltaTime);

    void clearOtherAvatars();
    void clearAllAvatars();

    bool shouldShowReceiveStats() const { return _shouldShowReceiveStats; }

    class LocalLight {
    public:
        glm::vec3 color;
        glm::vec3 direction;
    };

    Q_INVOKABLE void setLocalLights(const QVector<AvatarManager::LocalLight>& localLights);
    Q_INVOKABLE QVector<AvatarManager::LocalLight> getLocalLights() const;


    void getObjectsToRemoveFromPhysics(VectorOfMotionStates& motionStates);
    void getObjectsToAddToPhysics(VectorOfMotionStates& motionStates);
    void getObjectsToChange(VectorOfMotionStates& motionStates);
    void handleOutgoingChanges(const VectorOfMotionStates& motionStates);
    void handleCollisionEvents(const CollisionEvents& collisionEvents);

    void addAvatarToSimulation(Avatar* avatar);

    Q_INVOKABLE RayToAvatarIntersectionResult findRayIntersection(const PickRay& ray,
                                                                  const QScriptValue& avatarIdsToInclude = QScriptValue(),
                                                                  const QScriptValue& avatarIdsToDiscard = QScriptValue());

    float getMyAvatarSendRate() const { return _myAvatarSendRate.rate(); }

public slots:
    void setShouldShowReceiveStats(bool shouldShowReceiveStats) { _shouldShowReceiveStats = shouldShowReceiveStats; }
    void updateAvatarRenderStatus(bool shouldRenderAvatars);

private slots:
    virtual void removeAvatar(const QUuid& sessionUUID, KillAvatarReason removalReason = KillAvatarReason::NoReason) override;

private:
    explicit AvatarManager(QObject* parent = 0);
    explicit AvatarManager(const AvatarManager& other);

    void simulateAvatarFades(float deltaTime);

    // virtual overrides
    virtual AvatarSharedPointer newSharedAvatar() override;
    virtual AvatarSharedPointer addAvatar(const QUuid& sessionUUID, const QWeakPointer<Node>& mixerWeakPointer) override;
    virtual void handleRemovedAvatar(const AvatarSharedPointer& removedAvatar, KillAvatarReason removalReason = KillAvatarReason::NoReason) override;

    QVector<AvatarSharedPointer> _avatarFades;
    std::shared_ptr<MyAvatar> _myAvatar;
    quint64 _lastSendAvatarDataTime = 0; // Controls MyAvatar send data rate.

    QVector<AvatarManager::LocalLight> _localLights;

    bool _shouldShowReceiveStats = false;

    std::list<QPointer<AudioInjector>> _collisionInjectors;

    SetOfAvatarMotionStates _motionStatesThatMightUpdate;
    SetOfMotionStates _motionStatesToAddToPhysics;
    VectorOfMotionStates _motionStatesToRemoveFromPhysics;

    RateCounter<> _myAvatarSendRate;

};

Q_DECLARE_METATYPE(AvatarManager::LocalLight)
Q_DECLARE_METATYPE(QVector<AvatarManager::LocalLight>)

#endif // hifi_AvatarManager_h
