//
//  EntityMotionState.cpp
//  libraries/entities/src
//
//  Created by Andrew Meadows on 2014.11.06
//  Copyright 2013 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <glm/gtx/norm.hpp>

#include <EntityItem.h>
#include <EntityItemProperties.h>
#include <EntityEditPacketSender.h>
#include <PhysicsCollisionGroups.h>

#include "BulletUtil.h"
#include "EntityMotionState.h"
#include "PhysicsEngine.h"
#include "PhysicsHelpers.h"
#include "PhysicsLogging.h"

#ifdef WANT_DEBUG_ENTITY_TREE_LOCKS
#include "EntityTree.h"
#endif

const uint8_t LOOPS_FOR_SIMULATION_ORPHAN = 50;
const quint64 USECS_BETWEEN_OWNERSHIP_BIDS = USECS_PER_SECOND / 5;

#ifdef WANT_DEBUG_ENTITY_TREE_LOCKS
bool EntityMotionState::entityTreeIsLocked() const {
    EntityTreeElementPointer element = _entity->getElement();
    EntityTreePointer tree = element ? element->getTree() : nullptr;
    if (!tree) {
        return true;
    }
    return true;
}
#else
bool entityTreeIsLocked() {
    return true;
}
#endif


EntityMotionState::EntityMotionState(btCollisionShape* shape, EntityItemPointer entity) :
    ObjectMotionState(shape),
    _entityPtr(entity),
    _entity(entity.get()),
    _serverPosition(0.0f),
    _serverRotation(),
    _serverVelocity(0.0f),
    _serverAngularVelocity(0.0f),
    _serverGravity(0.0f),
    _serverAcceleration(0.0f),
    _serverActionData(QByteArray()),
    _lastVelocity(glm::vec3(0.0f)),
    _measuredAcceleration(glm::vec3(0.0f)),
    _nextOwnershipBid(0),
    _measuredDeltaTime(0.0f),
    _lastMeasureStep(0),
    _lastStep(0),
    _loopsWithoutOwner(0),
    _accelerationNearlyGravityCount(0),
    _numInactiveUpdates(1),
    _outgoingPriority(ZERO_SIMULATION_PRIORITY)
{
    _type = MOTIONSTATE_TYPE_ENTITY;
    assert(_entity);
    assert(entityTreeIsLocked());
    setMass(_entity->computeMass());
}

EntityMotionState::~EntityMotionState() {
    assert(_entity);
    _entity = nullptr;
}

void EntityMotionState::updateServerPhysicsVariables() {
    assert(entityTreeIsLocked());
    if (_entity->getSimulatorID() == Physics::getSessionUUID()) {
        // don't slam these values if we are the simulation owner
        return;
    }

    _serverPosition = _entity->getPosition();
    _serverRotation = _entity->getRotation();
    _serverVelocity = _entity->getVelocity();
    _serverAngularVelocity = _entity->getAngularVelocity();
    _serverAcceleration = _entity->getAcceleration();
    _serverActionData = _entity->getActionData();
}

// virtual
bool EntityMotionState::handleEasyChanges(uint32_t& flags) {
    assert(entityTreeIsLocked());
    updateServerPhysicsVariables();
    ObjectMotionState::handleEasyChanges(flags);

    if (flags & Simulation::DIRTY_SIMULATOR_ID) {
        if (_entity->getSimulatorID().isNull()) {
            // simulation ownership has been removed by an external simulator
            if (glm::length2(_entity->getVelocity()) == 0.0f) {
                // this object is coming to rest --> clear the ACTIVATION flag and outgoing priority
                flags &= ~Simulation::DIRTY_PHYSICS_ACTIVATION;
                _body->setActivationState(WANTS_DEACTIVATION);
                _outgoingPriority = ZERO_SIMULATION_PRIORITY;
                _loopsWithoutOwner = 0;
            } else {
                // unowned object is still moving --> we should volunteer to own it
                // TODO? put a delay in here proportional to distance from object?
                setOutgoingPriority(VOLUNTEER_SIMULATION_PRIORITY);
                _loopsWithoutOwner = LOOPS_FOR_SIMULATION_ORPHAN;
                _nextOwnershipBid = 0;
            }
        } else {
            // this entity's simulation is owned by someone, so we push its ownership expiry into the future
            _nextOwnershipBid = usecTimestampNow() + USECS_BETWEEN_OWNERSHIP_BIDS;
            if (Physics::getSessionUUID() == _entity->getSimulatorID() || _entity->getSimulationPriority() >= _outgoingPriority) {
                // either we already own the simulation or our old outgoing priority momentarily looses to current owner
                // so we clear it
                _outgoingPriority = ZERO_SIMULATION_PRIORITY;
            }
        }
    }
    if (flags & Simulation::DIRTY_SIMULATOR_OWNERSHIP) {
        // The DIRTY_SIMULATOR_OWNERSHIP bit really means "we should bid for ownership at SCRIPT priority".
        // Since that bit is set there must be a local script that is updating the physics properties of the objects
        // therefore we upgrade _outgoingPriority to trigger a bid for ownership.
        setOutgoingPriority(SCRIPT_EDIT_SIMULATION_PRIORITY);
    }
    if ((flags & Simulation::DIRTY_PHYSICS_ACTIVATION) && !_body->isActive()) {
        _body->activate();
    }

    return true;
}


// virtual
bool EntityMotionState::handleHardAndEasyChanges(uint32_t& flags, PhysicsEngine* engine) {
    updateServerPhysicsVariables();
    return ObjectMotionState::handleHardAndEasyChanges(flags, engine);
}

PhysicsMotionType EntityMotionState::computePhysicsMotionType() const {
    if (!_entity) {
        return MOTION_TYPE_STATIC;
    }
    assert(entityTreeIsLocked());

    if (_entity->getDynamic()) {
        if (!_entity->getParentID().isNull()) {
            // if something would have been dynamic but is a child of something else, force it to be kinematic, instead.
            return MOTION_TYPE_KINEMATIC;
        }
        return MOTION_TYPE_DYNAMIC;
    }
    return (_entity->isMovingRelativeToParent() || _entity->hasActions()) ?  MOTION_TYPE_KINEMATIC : MOTION_TYPE_STATIC;
}

bool EntityMotionState::isMoving() const {
    assert(entityTreeIsLocked());
    return _entity && _entity->isMovingRelativeToParent();
}

// This callback is invoked by the physics simulation in two cases:
// (1) when the RigidBody is first added to the world
//     (irregardless of PhysicsMotionType: STATIC, DYNAMIC, or KINEMATIC)
// (2) at the beginning of each simulation step for KINEMATIC RigidBody's --
//     it is an opportunity for outside code to update the object's simulation position
void EntityMotionState::getWorldTransform(btTransform& worldTrans) const {
    if (!_entity) {
        return;
    }
    assert(entityTreeIsLocked());
    if (_motionType == MOTION_TYPE_KINEMATIC) {
        // This is physical kinematic motion which steps strictly by the subframe count
        // of the physics simulation.
        uint32_t thisStep = ObjectMotionState::getWorldSimulationStep();
        float dt = (thisStep - _lastKinematicStep) * PHYSICS_ENGINE_FIXED_SUBSTEP;
        _entity->simulateKinematicMotion(dt);

        // bypass const-ness so we can remember the step
        const_cast<EntityMotionState*>(this)->_lastKinematicStep = thisStep;
    }
    worldTrans.setOrigin(glmToBullet(getObjectPosition()));
    worldTrans.setRotation(glmToBullet(_entity->getRotation()));
}

// This callback is invoked by the physics simulation at the end of each simulation step...
// iff the corresponding RigidBody is DYNAMIC and has moved.
void EntityMotionState::setWorldTransform(const btTransform& worldTrans) {
    if (!_entity) {
        return;
    }

    assert(entityTreeIsLocked());
    measureBodyAcceleration();
    _entity->setPosition(bulletToGLM(worldTrans.getOrigin()) + ObjectMotionState::getWorldOffset());
    _entity->setRotation(bulletToGLM(worldTrans.getRotation()));
    _entity->setVelocity(getBodyLinearVelocity());
    _entity->setAngularVelocity(getBodyAngularVelocity());
    _entity->setLastSimulated(usecTimestampNow());

    if (_entity->getSimulatorID().isNull()) {
        _loopsWithoutOwner++;

        if (_loopsWithoutOwner > LOOPS_FOR_SIMULATION_ORPHAN && usecTimestampNow() > _nextOwnershipBid) {
            setOutgoingPriority(VOLUNTEER_SIMULATION_PRIORITY);
        }
    }

    #ifdef WANT_DEBUG
        quint64 now = usecTimestampNow();
        qCDebug(physics) << "EntityMotionState::setWorldTransform()... changed entity:" << _entity->getEntityItemID();
        qCDebug(physics) << "       last edited:" << _entity->getLastEdited()
                         << formatUsecTime(now - _entity->getLastEdited()) << "ago";
        qCDebug(physics) << "    last simulated:" << _entity->getLastSimulated()
                         << formatUsecTime(now - _entity->getLastSimulated()) << "ago";
        qCDebug(physics) << "      last updated:" << _entity->getLastUpdated()
                         << formatUsecTime(now - _entity->getLastUpdated()) << "ago";
    #endif
}


// virtual and protected
bool EntityMotionState::isReadyToComputeShape() const {
    return _entity->isReadyToComputeShape();
}

// virtual and protected
btCollisionShape* EntityMotionState::computeNewShape() {
    ShapeInfo shapeInfo;
    assert(entityTreeIsLocked());
    _entity->computeShapeInfo(shapeInfo);
    return getShapeManager()->getShape(shapeInfo);
}

bool EntityMotionState::isCandidateForOwnership(const QUuid& sessionID) const {
    assert(_body);
    assert(_entity);
    assert(entityTreeIsLocked());
    return _outgoingPriority != ZERO_SIMULATION_PRIORITY || sessionID == _entity->getSimulatorID() || _entity->actionDataNeedsTransmit();
}

bool EntityMotionState::remoteSimulationOutOfSync(uint32_t simulationStep) {
    // NOTE: we only get here if we think we own the simulation
    assert(_body);
    // if we've never checked before, our _lastStep will be 0, and we need to initialize our state
    if (_lastStep == 0) {
        btTransform xform = _body->getWorldTransform();
        _serverPosition = bulletToGLM(xform.getOrigin());
        _serverRotation = bulletToGLM(xform.getRotation());
        _serverVelocity = getBodyLinearVelocityGTSigma();
        _serverAngularVelocity = bulletToGLM(_body->getAngularVelocity());
        _lastStep = simulationStep;
        _serverActionData = _entity->getActionData();
        _numInactiveUpdates = 1;
        return false;
    }

    #ifdef WANT_DEBUG
    glm::vec3 wasPosition = _serverPosition;
    glm::quat wasRotation = _serverRotation;
    glm::vec3 wasAngularVelocity = _serverAngularVelocity;
    #endif

    int numSteps = simulationStep - _lastStep;
    float dt = (float)(numSteps) * PHYSICS_ENGINE_FIXED_SUBSTEP;

    if (_numInactiveUpdates > 0) {
        const uint8_t MAX_NUM_INACTIVE_UPDATES = 3;
        if (_numInactiveUpdates > MAX_NUM_INACTIVE_UPDATES) {
            // clear local ownership (stop sending updates) and let the server clear itself
            _entity->clearSimulationOwnership();
            return false;
        }
        // we resend the inactive update every INACTIVE_UPDATE_PERIOD
        // until it is removed from the outgoing updates
        // (which happens when we don't own the simulation and it isn't touching our simulation)
        const float INACTIVE_UPDATE_PERIOD = 0.5f;
        return (dt > INACTIVE_UPDATE_PERIOD);
    }

    if (!_body->isActive()) {
        // object has gone inactive but our last send was moving --> send non-moving update immediately
        return true;
    }

    _lastStep = simulationStep;
    if (glm::length2(_serverVelocity) > 0.0f) {
        _serverVelocity += _serverAcceleration * dt;
        _serverVelocity *= powf(1.0f - _body->getLinearDamping(), dt);
        _serverPosition += dt * _serverVelocity;
    }

    if (_entity->actionDataNeedsTransmit()) {
        setOutgoingPriority(SCRIPT_EDIT_SIMULATION_PRIORITY);
        return true;
    }

    if (_entity->shouldSuppressLocationEdits()) {
        return false;
    }

    // Else we measure the error between current and extrapolated transform (according to expected behavior
    // of remote EntitySimulation) and return true if the error is significant.

    // NOTE: math is done in the simulation-frame, which is NOT necessarily the same as the world-frame
    // due to _worldOffset.
    // TODO: compensate for _worldOffset offset here

    // compute position error

    btTransform worldTrans = _body->getWorldTransform();
    glm::vec3 position = bulletToGLM(worldTrans.getOrigin());

    float dx2 = glm::distance2(position, _serverPosition);

    const float MAX_POSITION_ERROR_SQUARED = 0.000004f; //  Sqrt() - corresponds to 2 millimeters
    if (dx2 > MAX_POSITION_ERROR_SQUARED) {

        #ifdef WANT_DEBUG
            qCDebug(physics) << ".... (dx2 > MAX_POSITION_ERROR_SQUARED) ....";
            qCDebug(physics) << "wasPosition:" << wasPosition;
            qCDebug(physics) << "bullet position:" << position;
            qCDebug(physics) << "_serverPosition:" << _serverPosition;
            qCDebug(physics) << "dx2:" << dx2;
        #endif

        return true;
    }

    if (glm::length2(_serverAngularVelocity) > 0.0f) {
        // compute rotation error
        float attenuation = powf(1.0f - _body->getAngularDamping(), dt);
        _serverAngularVelocity *= attenuation;

        // Bullet caps the effective rotation velocity inside its rotation integration step, therefore
        // we must integrate with the same algorithm and timestep in order achieve similar results.
        for (int i = 0; i < numSteps; ++i) {
            _serverRotation = glm::normalize(computeBulletRotationStep(_serverAngularVelocity,
                                                                       PHYSICS_ENGINE_FIXED_SUBSTEP) * _serverRotation);
        }
    }
    const float MIN_ROTATION_DOT = 0.99999f; // This corresponds to about 0.5 degrees of rotation
    glm::quat actualRotation = bulletToGLM(worldTrans.getRotation());

    #ifdef WANT_DEBUG
        if ((fabsf(glm::dot(actualRotation, _serverRotation)) < MIN_ROTATION_DOT)) {
            qCDebug(physics) << ".... ((fabsf(glm::dot(actualRotation, _serverRotation)) < MIN_ROTATION_DOT)) ....";

            qCDebug(physics) << "wasAngularVelocity:" << wasAngularVelocity;
            qCDebug(physics) << "_serverAngularVelocity:" << _serverAngularVelocity;

            qCDebug(physics) << "length wasAngularVelocity:" << glm::length(wasAngularVelocity);
            qCDebug(physics) << "length _serverAngularVelocity:" << glm::length(_serverAngularVelocity);

            qCDebug(physics) << "wasRotation:" << wasRotation;
            qCDebug(physics) << "bullet actualRotation:" << actualRotation;
            qCDebug(physics) << "_serverRotation:" << _serverRotation;
        }
    #endif

    return (fabsf(glm::dot(actualRotation, _serverRotation)) < MIN_ROTATION_DOT);
}

bool EntityMotionState::shouldSendUpdate(uint32_t simulationStep, const QUuid& sessionID) {
    // NOTE: we expect _entity and _body to be valid in this context, since shouldSendUpdate() is only called
    // after doesNotNeedToSendUpdate() returns false and that call should return 'true' if _entity or _body are NULL.
    assert(_entity);
    assert(_body);
    assert(entityTreeIsLocked());

    if (_entity->actionDataNeedsTransmit()) {
        return true;
    }

    if (_entity->queryAABoxNeedsUpdate()) {
        return true;
    }

    if (_entity->getSimulatorID() != sessionID) {
        // we don't own the simulation
        if (_outgoingPriority != ZERO_SIMULATION_PRIORITY) {
            // but we would like to own it
            if (_outgoingPriority < _entity->getSimulationPriority()) {
                // but our priority loses to remote, so we don't bother trying
                _outgoingPriority = ZERO_SIMULATION_PRIORITY;
                return false;
            }
            return usecTimestampNow() > _nextOwnershipBid;
        }
        return false;
    }

    return remoteSimulationOutOfSync(simulationStep);
}

void EntityMotionState::sendUpdate(OctreeEditPacketSender* packetSender, const QUuid& sessionID, uint32_t step) {
    assert(_entity);
    assert(entityTreeIsLocked());

    bool active = _body->isActive();
    if (!active) {
        // make sure all derivatives are zero
        glm::vec3 zero(0.0f);
        _entity->setVelocity(zero);
        _entity->setAngularVelocity(zero);
        _entity->setAcceleration(zero);
        _numInactiveUpdates++;
    } else {
        const uint8_t STEPS_TO_DECIDE_BALLISTIC = 4;
        float gravityLength = glm::length(_entity->getGravity());
        float accVsGravity = glm::abs(glm::length(_measuredAcceleration) - gravityLength);
        const float ACCELERATION_EQUIVALENT_EPSILON_RATIO = 0.1f;
        if (accVsGravity < ACCELERATION_EQUIVALENT_EPSILON_RATIO * gravityLength) {
            // acceleration measured during the most recent simulation step was close to gravity.
            if (getAccelerationNearlyGravityCount() < STEPS_TO_DECIDE_BALLISTIC) {
                // only increment this if we haven't reached the threshold yet.  this is to avoid
                // overflowing the counter.
                incrementAccelerationNearlyGravityCount();
            }
        } else {
            // acceleration wasn't similar to this entities gravity, so reset the went-ballistic counter
            resetAccelerationNearlyGravityCount();
        }

        // if this entity has been accelerated at close to gravity for a certain number of simulation-steps, let
        // the entity server's estimates include gravity.
        if (getAccelerationNearlyGravityCount() >= STEPS_TO_DECIDE_BALLISTIC) {
            _entity->setAcceleration(_entity->getGravity());
        } else {
            _entity->setAcceleration(glm::vec3(0.0f));
        }

        const float DYNAMIC_LINEAR_VELOCITY_THRESHOLD = 0.05f;  // 5 cm/sec
        const float DYNAMIC_ANGULAR_VELOCITY_THRESHOLD = 0.087266f;  // ~5 deg/sec

        bool movingSlowlyLinear =
            glm::length2(_entity->getVelocity()) < (DYNAMIC_LINEAR_VELOCITY_THRESHOLD * DYNAMIC_LINEAR_VELOCITY_THRESHOLD);
        bool movingSlowlyAngular = glm::length2(_entity->getAngularVelocity()) <
                (DYNAMIC_ANGULAR_VELOCITY_THRESHOLD * DYNAMIC_ANGULAR_VELOCITY_THRESHOLD);
        bool movingSlowly = movingSlowlyLinear && movingSlowlyAngular && _entity->getAcceleration() == glm::vec3(0.0f);

        if (movingSlowly) {
            // velocities might not be zero, but we'll fake them as such, which will hopefully help convince
            // other simulating observers to deactivate their own copies
            glm::vec3 zero(0.0f);
            _entity->setVelocity(zero);
            _entity->setAngularVelocity(zero);
        }
        _numInactiveUpdates = 0;
    }

    // remember properties for local server prediction
    _serverPosition = _entity->getPosition();
    _serverRotation = _entity->getRotation();
    _serverVelocity = _entity->getVelocity();
    _serverAcceleration = _entity->getAcceleration();
    _serverAngularVelocity = _entity->getAngularVelocity();
    _serverActionData = _entity->getActionData();

    EntityItemProperties properties;

    // explicitly set the properties that changed so that they will be packed
    properties.setPosition(_entity->getLocalPosition());
    properties.setRotation(_entity->getLocalOrientation());

    properties.setVelocity(_serverVelocity);
    properties.setAcceleration(_serverAcceleration);
    properties.setAngularVelocity(_serverAngularVelocity);
    if (_entity->actionDataNeedsTransmit()) {
        _entity->setActionDataNeedsTransmit(false);
        properties.setActionData(_serverActionData);
    }

    if (properties.parentRelatedPropertyChanged() && _entity->computePuffedQueryAACube()) {
        // due to parenting, the server may not know where something is in world-space, so include the bounding cube.
        properties.setQueryAACube(_entity->getQueryAACube());
    }

    // set the LastEdited of the properties but NOT the entity itself
    quint64 now = usecTimestampNow();
    properties.setLastEdited(now);

    #ifdef WANT_DEBUG
        quint64 lastSimulated = _entity->getLastSimulated();
        qCDebug(physics) << "EntityMotionState::sendUpdate()";
        qCDebug(physics) << "        EntityItemId:" << _entity->getEntityItemID()
                         << "---------------------------------------------";
        qCDebug(physics) << "       lastSimulated:" << debugTime(lastSimulated, now);
    #endif //def WANT_DEBUG

    if (sessionID == _entity->getSimulatorID()) {
        // we think we own the simulation
        if (!active) {
            // we own the simulation but the entity has stopped, so we tell the server that we're clearing simulatorID
            // but we remember that we do still own it...  and rely on the server to tell us that we don't
            properties.clearSimulationOwner();
            _outgoingPriority = ZERO_SIMULATION_PRIORITY;
        }
        // else the ownership is not changing so we don't bother to pack it
    } else {
        // we don't own the simulation for this entity yet, but we're sending a bid for it
        properties.setSimulationOwner(sessionID, glm::max<uint8_t>(_outgoingPriority, VOLUNTEER_SIMULATION_PRIORITY));
        _nextOwnershipBid = now + USECS_BETWEEN_OWNERSHIP_BIDS;
    }

    EntityItemID id(_entity->getID());
    EntityEditPacketSender* entityPacketSender = static_cast<EntityEditPacketSender*>(packetSender);
    #ifdef WANT_DEBUG
        qCDebug(physics) << "EntityMotionState::sendUpdate()... calling queueEditEntityMessage()...";
    #endif

    entityPacketSender->queueEditEntityMessage(PacketType::EntityEdit, id, properties);
    _entity->setLastBroadcast(usecTimestampNow());

    // if we've moved an entity with children, check/update the queryAACube of all descendents and tell the server
    // if they've changed.
    _entity->forEachDescendant([&](SpatiallyNestablePointer descendant) {
        if (descendant->getNestableType() == NestableType::Entity) {
            EntityItemPointer entityDescendant = std::static_pointer_cast<EntityItem>(descendant);
            if (descendant->computePuffedQueryAACube()) {
                EntityItemProperties newQueryCubeProperties;
                newQueryCubeProperties.setQueryAACube(descendant->getQueryAACube());
                newQueryCubeProperties.setLastEdited(properties.getLastEdited());
                entityPacketSender->queueEditEntityMessage(PacketType::EntityEdit, descendant->getID(), newQueryCubeProperties);
                entityDescendant->setLastBroadcast(usecTimestampNow());
            }
        }
    });

    _lastStep = step;
}

uint32_t EntityMotionState::getIncomingDirtyFlags() {
    assert(entityTreeIsLocked());
    uint32_t dirtyFlags = 0;
    if (_body && _entity) {
        dirtyFlags = _entity->getDirtyFlags();

        if (dirtyFlags | Simulation::DIRTY_SIMULATOR_ID) {
            // when SIMULATOR_ID changes we must check for reinterpretation of asymmetric collision mask
            // bits for the avatar groups (e.g. MY_AVATAR vs OTHER_AVATAR)
            uint8_t entityCollisionMask = _entity->getCollisionless() ? 0 : _entity->getCollisionMask();
            if ((bool)(entityCollisionMask & USER_COLLISION_GROUP_MY_AVATAR) !=
                    (bool)(entityCollisionMask & USER_COLLISION_GROUP_OTHER_AVATAR)) {
                // bits are asymmetric --> flag for reinsertion in physics simulation
                dirtyFlags |= Simulation::DIRTY_COLLISION_GROUP;
            }
        }
        // we add DIRTY_MOTION_TYPE if the body's motion type disagrees with entity velocity settings
        int bodyFlags = _body->getCollisionFlags();
        bool isMoving = _entity->isMovingRelativeToParent();
        if (((bodyFlags & btCollisionObject::CF_STATIC_OBJECT) && isMoving) ||
                (bodyFlags & btCollisionObject::CF_KINEMATIC_OBJECT && !isMoving)) {
            dirtyFlags |= Simulation::DIRTY_MOTION_TYPE;
        }
    }
    return dirtyFlags;
}

void EntityMotionState::clearIncomingDirtyFlags() {
    assert(entityTreeIsLocked());
    if (_body && _entity) {
        _entity->clearDirtyFlags();
    }
}

// virtual
uint8_t EntityMotionState::getSimulationPriority() const {
    return _entity->getSimulationPriority();
}

// virtual
QUuid EntityMotionState::getSimulatorID() const {
    assert(entityTreeIsLocked());
    return _entity->getSimulatorID();
}

void EntityMotionState::bump(uint8_t priority) {
    setOutgoingPriority(glm::max(VOLUNTEER_SIMULATION_PRIORITY, --priority));
}

void EntityMotionState::resetMeasuredBodyAcceleration() {
    _lastMeasureStep = ObjectMotionState::getWorldSimulationStep();
    if (_body) {
        _lastVelocity = getBodyLinearVelocityGTSigma();
    } else {
        _lastVelocity = glm::vec3(0.0f);
    }
    _measuredAcceleration = glm::vec3(0.0f);
}

void EntityMotionState::measureBodyAcceleration() {
    // try to manually measure the true acceleration of the object
    uint32_t thisStep = ObjectMotionState::getWorldSimulationStep();
    uint32_t numSubsteps = thisStep - _lastMeasureStep;
    if (numSubsteps > 0) {
        float dt = ((float)numSubsteps * PHYSICS_ENGINE_FIXED_SUBSTEP);
        float invDt = 1.0f / dt;
        _lastMeasureStep = thisStep;
        _measuredDeltaTime = dt;

        // Note: the integration equation for velocity uses damping:   v1 = (v0 + a * dt) * (1 - D)^dt
        // hence the equation for acceleration is: a = (v1 / (1 - D)^dt - v0) / dt
        glm::vec3 velocity = getBodyLinearVelocityGTSigma();

        _measuredAcceleration = (velocity / powf(1.0f - _body->getLinearDamping(), dt) - _lastVelocity) * invDt;
        _lastVelocity = velocity;
        if (numSubsteps > PHYSICS_ENGINE_MAX_NUM_SUBSTEPS) {
            _loopsWithoutOwner = 0;
            _lastStep = ObjectMotionState::getWorldSimulationStep();
            _numInactiveUpdates = 0;
        }
    }
}
glm::vec3 EntityMotionState::getObjectLinearVelocityChange() const {
    // This is the dampened change in linear velocity, as calculated in measureBodyAcceleration: dv = a * dt
    // It is generally only meaningful during the lifespan of collision. In particular, it is not meaningful
    // when the entity first starts moving via direct user action.
    return _measuredAcceleration * _measuredDeltaTime;
}

// virtual
void EntityMotionState::setMotionType(PhysicsMotionType motionType) {
    ObjectMotionState::setMotionType(motionType);
    resetMeasuredBodyAcceleration();
}


// virtual
QString EntityMotionState::getName() const {
    assert(entityTreeIsLocked());
    return _entity->getName();
}

// virtual
void EntityMotionState::computeCollisionGroupAndMask(int16_t& group, int16_t& mask) const {
    assert(_entity);
    _entity->computeCollisionGroupAndFinalMask(group, mask);
}

void EntityMotionState::setOutgoingPriority(uint8_t priority) {
    _outgoingPriority = glm::max<uint8_t>(_outgoingPriority, priority);
}
