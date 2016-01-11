//
//  SpatiallyNestable.cpp
//  libraries/shared/src/
//
//  Created by Seth Alves on 2015-10-18
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include <QQueue>

#include "DependencyManager.h"
#include "SpatiallyNestable.h"

const float defaultAACubeSize = 1.0f;

SpatiallyNestable::SpatiallyNestable(NestableType nestableType, QUuid id) :
    _nestableType(nestableType),
    _id(id),
    _transform() {
    // set flags in _transform
    _transform.setTranslation(glm::vec3(0.0f));
    _transform.setRotation(glm::quat());
}

Transform SpatiallyNestable::getParentTransform(bool& success) const {
    Transform result;
    SpatiallyNestablePointer parent = getParentPointer(success);
    if (!success) {
        return result;
    }
    if (parent) {
        Transform parentTransform = parent->getTransform(_parentJointIndex, success);
        result = parentTransform.setScale(1.0f); // TODO: scaling
    }
    return result;
}

SpatiallyNestablePointer SpatiallyNestable::getParentPointer(bool& success) const {
    SpatiallyNestablePointer parent = _parent.lock();

    if (!parent && _parentID.isNull()) {
        // no parent
        success = true;
        return nullptr;
    }

    if (parent && parent->getID() == _parentID) {
        // parent pointer is up-to-date
        if (!_parentKnowsMe) {
            parent->beParentOfChild(getThisPointer());
            _parentKnowsMe = true;
        }
        success = true;
        return parent;
    }

    SpatiallyNestablePointer thisPointer = getThisPointer();

    if (parent) {
        // we have a parent pointer but our _parentID doesn't indicate this parent.
        parent->forgetChild(thisPointer);
        _parentKnowsMe = false;
        _parent.reset();
    }

    // we have a _parentID but no parent pointer, or our parent pointer was to the wrong thing
    QSharedPointer<SpatialParentFinder> parentFinder = DependencyManager::get<SpatialParentFinder>();
    if (!parentFinder) {
        success = false;
        return nullptr;
    }
    _parent = parentFinder->find(_parentID, success);
    if (!success) {
        return nullptr;
    }

    parent = _parent.lock();
    if (parent) {
        parent->beParentOfChild(thisPointer);
        _parentKnowsMe = true;
    }

    if (parent || _parentID.isNull()) {
        success = true;
    } else {
        success = false;
    }

    return parent;
}

void SpatiallyNestable::beParentOfChild(SpatiallyNestablePointer newChild) const {
    _childrenLock.withWriteLock([&] {
        _children[newChild->getID()] = newChild;
    });
}

void SpatiallyNestable::forgetChild(SpatiallyNestablePointer newChild) const {
    _childrenLock.withWriteLock([&] {
        _children.remove(newChild->getID());
    });
}

void SpatiallyNestable::setParentID(const QUuid& parentID) {
    if (_parentID != parentID) {
        _parentID = parentID;
        _parentKnowsMe = false;
    }
}

void SpatiallyNestable::setParentJointIndex(quint16 parentJointIndex) {
    _parentJointIndex = parentJointIndex;
}

glm::vec3 SpatiallyNestable::worldToLocal(const glm::vec3& position,
                                          const QUuid& parentID, int parentJointIndex,
                                          bool& success) {
    Transform result;
    QSharedPointer<SpatialParentFinder> parentFinder = DependencyManager::get<SpatialParentFinder>();
    if (!parentFinder) {
        success = false;
        return glm::vec3(0.0f);
    }

    Transform parentTransform;
    auto parentWP = parentFinder->find(parentID, success);
    if (!success) {
        return glm::vec3(0.0f);
    }

    auto parent = parentWP.lock();
    if (!parentID.isNull() && !parent) {
        success = false;
        return glm::vec3(0.0f);
    }

    if (parent) {
        parentTransform = parent->getTransform(parentJointIndex, success);
        if (!success) {
            return glm::vec3(0.0f);
        }
        parentTransform.setScale(1.0f); // TODO: scale
    }
    success = true;

    Transform positionTransform;
    positionTransform.setTranslation(position);
    Transform myWorldTransform;
    Transform::mult(myWorldTransform, parentTransform, positionTransform);
    myWorldTransform.setTranslation(position);
    Transform::inverseMult(result, parentTransform, myWorldTransform);
    return result.getTranslation();
}

glm::quat SpatiallyNestable::worldToLocal(const glm::quat& orientation,
                                          const QUuid& parentID, int parentJointIndex,
                                          bool& success) {
    Transform result;
    QSharedPointer<SpatialParentFinder> parentFinder = DependencyManager::get<SpatialParentFinder>();
    if (!parentFinder) {
        success = false;
        return glm::quat();
    }

    Transform parentTransform;
    auto parentWP = parentFinder->find(parentID, success);
    if (!success) {
        return glm::quat();
    }

    auto parent = parentWP.lock();
    if (!parentID.isNull() && !parent) {
        success = false;
        return glm::quat();
    }

    if (parent) {
        parentTransform = parent->getTransform(parentJointIndex, success);
        if (!success) {
            return glm::quat();
        }
        parentTransform.setScale(1.0f); // TODO: scale
    }
    success = true;

    Transform orientationTransform;
    orientationTransform.setRotation(orientation);
    Transform myWorldTransform;
    Transform::mult(myWorldTransform, parentTransform, orientationTransform);
    myWorldTransform.setRotation(orientation);
    Transform::inverseMult(result, parentTransform, myWorldTransform);
    return result.getRotation();
}

glm::vec3 SpatiallyNestable::localToWorld(const glm::vec3& position,
                                          const QUuid& parentID, int parentJointIndex,
                                          bool& success) {
    Transform result;
    QSharedPointer<SpatialParentFinder> parentFinder = DependencyManager::get<SpatialParentFinder>();
    if (!parentFinder) {
        success = false;
        return glm::vec3(0.0f);
    }

    Transform parentTransform;
    auto parentWP = parentFinder->find(parentID, success);
    if (!success) {
        return glm::vec3(0.0f);
    }

    auto parent = parentWP.lock();
    if (!parentID.isNull() && !parent) {
        success = false;
        return glm::vec3(0.0f);
    }

    if (parent) {
        parentTransform = parent->getTransform(parentJointIndex, success);
        if (!success) {
            return glm::vec3(0.0f);
        }
        parentTransform.setScale(1.0f); // TODO: scale
    }
    success = true;

    Transform positionTransform;
    positionTransform.setTranslation(position);
    Transform::mult(result, parentTransform, positionTransform);
    return result.getTranslation();
}

glm::quat SpatiallyNestable::localToWorld(const glm::quat& orientation,
                                          const QUuid& parentID, int parentJointIndex,
                                          bool& success) {
    Transform result;
    QSharedPointer<SpatialParentFinder> parentFinder = DependencyManager::get<SpatialParentFinder>();
    if (!parentFinder) {
        success = false;
        return glm::quat();
    }

    Transform parentTransform;
    auto parentWP = parentFinder->find(parentID, success);
    if (!success) {
        return glm::quat();
    }

    auto parent = parentWP.lock();
    if (!parentID.isNull() && !parent) {
        success = false;
        return glm::quat();
    }

    if (parent) {
        parentTransform = parent->getTransform(parentJointIndex, success);
        if (!success) {
            return glm::quat();
        }
        parentTransform.setScale(1.0f);
    }
    success = true;

    Transform orientationTransform;
    orientationTransform.setRotation(orientation);
    Transform::mult(result, parentTransform, orientationTransform);
    return result.getRotation();
}

glm::vec3 SpatiallyNestable::getPosition(bool& success) const {
    return getTransform(success).getTranslation();
}

glm::vec3 SpatiallyNestable::getPosition() const {
    bool success;
    auto result = getPosition(success);
    #ifdef WANT_DEBUG
    if (!success) {
        qDebug() << "Warning -- getPosition failed" << getID();
    }
    #endif
    return result;
}

glm::vec3 SpatiallyNestable::getPosition(int jointIndex, bool& success) const {
    return getTransform(jointIndex, success).getTranslation();
}

void SpatiallyNestable::setPosition(const glm::vec3& position, bool& success) {
    Transform parentTransform = getParentTransform(success);
    Transform myWorldTransform;
    _transformLock.withWriteLock([&] {
        Transform::mult(myWorldTransform, parentTransform, _transform);
        myWorldTransform.setTranslation(position);
        Transform::inverseMult(_transform, parentTransform, myWorldTransform);
    });
    if (success) {
        locationChanged();
    } else {
        qDebug() << "setPosition failed for" << getID();
    }
}

void SpatiallyNestable::setPosition(const glm::vec3& position) {
    bool success;
    setPosition(position, success);
    #ifdef WANT_DEBUG
    if (!success) {
        qDebug() << "Warning -- setPosition failed" << getID();
    }
    #endif
}

glm::quat SpatiallyNestable::getOrientation(bool& success) const {
    return getTransform(success).getRotation();
}

glm::quat SpatiallyNestable::getOrientation() const {
    bool success;
    auto result = getOrientation(success);
    #ifdef WANT_DEBUG
    if (!success) {
        qDebug() << "Warning -- getOrientation failed" << getID();
    }
    #endif
    return result;
}

glm::quat SpatiallyNestable::getOrientation(int jointIndex, bool& success) const {
    return getTransform(jointIndex, success).getRotation();
}

void SpatiallyNestable::setOrientation(const glm::quat& orientation, bool& success) {
    Transform parentTransform = getParentTransform(success);
    Transform myWorldTransform;
    _transformLock.withWriteLock([&] {
        Transform::mult(myWorldTransform, parentTransform, _transform);
        myWorldTransform.setRotation(orientation);
        Transform::inverseMult(_transform, parentTransform, myWorldTransform);
    });
    if (success) {
        locationChanged();
    }
}

void SpatiallyNestable::setOrientation(const glm::quat& orientation) {
    bool success;
    setOrientation(orientation, success);
    #ifdef WANT_DEBUG
    if (!success) {
        qDebug() << "Warning -- setOrientation failed" << getID();
    }
    #endif
}

const Transform SpatiallyNestable::getTransform(bool& success) const {
    // return a world-space transform for this object's location
    Transform parentTransform = getParentTransform(success);
    Transform result;
    _transformLock.withReadLock([&] {
        Transform::mult(result, parentTransform, _transform);
    });
    return result;
}

const Transform SpatiallyNestable::getTransform(int jointIndex, bool& success) const {
    // this returns the world-space transform for this object.  It finds its parent's transform (which may
    // cause this object's parent to query its parent, etc) and multiplies this object's local transform onto it.
    Transform jointInWorldFrame;

    Transform worldTransform = getTransform(success);
    if (!success) {
        return jointInWorldFrame;
    }

    Transform jointInObjectFrame = getAbsoluteJointTransformInObjectFrame(jointIndex);
    Transform::mult(jointInWorldFrame, worldTransform, jointInObjectFrame);
    success = true;
    return jointInWorldFrame;
}

void SpatiallyNestable::setTransform(const Transform& transform, bool& success) {
    Transform parentTransform = getParentTransform(success);
    _transformLock.withWriteLock([&] {
        Transform::inverseMult(_transform, parentTransform, transform);
    });
    if (success) {
        locationChanged();
    }
}

glm::vec3 SpatiallyNestable::getScale() const {
    // TODO: scale
    glm::vec3 result;
    _transformLock.withReadLock([&] {
        result = _transform.getScale();
    });
    return result;
}

glm::vec3 SpatiallyNestable::getScale(int jointIndex) const {
    // TODO: scale
    return getScale();
}

void SpatiallyNestable::setScale(const glm::vec3& scale) {
    // TODO: scale
    _transformLock.withWriteLock([&] {
        _transform.setScale(scale);
    });
    dimensionsChanged();
}

const Transform SpatiallyNestable::getLocalTransform() const {
    Transform result;
    _transformLock.withReadLock([&] {
        result =_transform;
    });
    return result;
}

void SpatiallyNestable::setLocalTransform(const Transform& transform) {
    _transformLock.withWriteLock([&] {
        _transform = transform;
    });
    locationChanged();
}

glm::vec3 SpatiallyNestable::getLocalPosition() const {
    glm::vec3 result;
    _transformLock.withReadLock([&] {
        result = _transform.getTranslation();
    });
    return result;
}

void SpatiallyNestable::setLocalPosition(const glm::vec3& position) {
    _transformLock.withWriteLock([&] {
        _transform.setTranslation(position);
    });
    locationChanged();
}

glm::quat SpatiallyNestable::getLocalOrientation() const {
    glm::quat result;
    _transformLock.withReadLock([&] {
        result = _transform.getRotation();
    });
    return result;
}

void SpatiallyNestable::setLocalOrientation(const glm::quat& orientation) {
    _transformLock.withWriteLock([&] {
        _transform.setRotation(orientation);
    });
    locationChanged();
}

glm::vec3 SpatiallyNestable::getLocalScale() const {
    // TODO: scale
    glm::vec3 result;
    _transformLock.withReadLock([&] {
        result = _transform.getScale();
    });
    return result;
}

void SpatiallyNestable::setLocalScale(const glm::vec3& scale) {
    // TODO: scale
    _transformLock.withWriteLock([&] {
        _transform.setScale(scale);
    });
    dimensionsChanged();
}

QList<SpatiallyNestablePointer> SpatiallyNestable::getChildren() const {
    QList<SpatiallyNestablePointer> children;
    _childrenLock.withReadLock([&] {
        foreach(SpatiallyNestableWeakPointer childWP, _children.values()) {
            SpatiallyNestablePointer child = childWP.lock();
            if (child) {
                children << child;
            }
        }
    });
    return children;
}

const Transform SpatiallyNestable::getAbsoluteJointTransformInObjectFrame(int jointIndex) const {
    Transform jointTransformInObjectFrame;
    glm::vec3 position = getAbsoluteJointTranslationInObjectFrame(jointIndex);
    glm::quat orientation = getAbsoluteJointRotationInObjectFrame(jointIndex);
    jointTransformInObjectFrame.setRotation(orientation);
    jointTransformInObjectFrame.setTranslation(position);
    return jointTransformInObjectFrame;
}

SpatiallyNestablePointer SpatiallyNestable::getThisPointer() const {
    SpatiallyNestableConstPointer constThisPointer = shared_from_this();
    SpatiallyNestablePointer thisPointer = std::const_pointer_cast<SpatiallyNestable>(constThisPointer); // ermahgerd !!!
    return thisPointer;
}

void SpatiallyNestable::forEachChild(std::function<void(SpatiallyNestablePointer)> actor) {
    foreach(SpatiallyNestablePointer child, getChildren()) {
        actor(child);
    }
}

void SpatiallyNestable::forEachDescendant(std::function<void(SpatiallyNestablePointer)> actor) {
    QQueue<SpatiallyNestablePointer> toProcess;
    foreach(SpatiallyNestablePointer child, getChildren()) {
        toProcess.enqueue(child);
    }

    while (!toProcess.empty()) {
        SpatiallyNestablePointer object = toProcess.dequeue();
        actor(object);
        foreach (SpatiallyNestablePointer child, object->getChildren()) {
            toProcess.enqueue(child);
        }
    }
}

void SpatiallyNestable::locationChanged() {
    forEachChild([&](SpatiallyNestablePointer object) {
        object->locationChanged();
    });
}

AACube SpatiallyNestable::getMaximumAACube(bool& success) const {
    return AACube(getPosition(success) - glm::vec3(defaultAACubeSize / 2.0f), defaultAACubeSize);
}

void SpatiallyNestable::setQueryAACube(const AACube& queryAACube) {
    _queryAACube = queryAACube;
    if (queryAACube.getScale() > 0.0f) {
        _queryAACubeSet = true;
    }
}

bool SpatiallyNestable::queryAABoxNeedsUpdate() const {
    bool success;
    AACube currentAACube = getMaximumAACube(success);
    if (!success) {
        qDebug() << "can't getMaximumAACube for" << getID();
        return false;
    }

    // make sure children are still in their boxes, also.
    bool childNeedsUpdate = false;
    getThisPointer()->forEachDescendant([&](SpatiallyNestablePointer descendant) {
        if (!childNeedsUpdate && descendant->queryAABoxNeedsUpdate()) {
            childNeedsUpdate = true;
        }
    });
    if (childNeedsUpdate) {
        return true;
    }

    if (_queryAACubeSet && _queryAACube.contains(currentAACube)) {
        return false;
    }

    return true;
}

bool SpatiallyNestable::computePuffedQueryAACube() {
    if (!queryAABoxNeedsUpdate()) {
        return false;
    }
    bool success;
    AACube currentAACube = getMaximumAACube(success);
    // make an AACube with edges thrice as long and centered on the object
    _queryAACube = AACube(currentAACube.getCorner() - glm::vec3(currentAACube.getScale()), currentAACube.getScale() * 3.0f);
    _queryAACubeSet = true;

    getThisPointer()->forEachDescendant([&](SpatiallyNestablePointer descendant) {
        bool success;
        AACube descendantAACube = descendant->getQueryAACube(success);
        if (success) {
            if (_queryAACube.contains(descendantAACube)) {
                return;
            }
            _queryAACube += descendantAACube.getMinimumPoint();
            _queryAACube += descendantAACube.getMaximumPoint();
        }
    });

    return true;
}

AACube SpatiallyNestable::getQueryAACube(bool& success) const {
    if (_queryAACubeSet) {
        success = true;
        return _queryAACube;
    }
    success = false;
    bool getPositionSuccess;
    return AACube(getPosition(getPositionSuccess) - glm::vec3(defaultAACubeSize / 2.0f), defaultAACubeSize);
}

AACube SpatiallyNestable::getQueryAACube() const {
    bool success;
    auto result = getQueryAACube(success);
    if (!success) {
        qDebug() << "getQueryAACube failed for" << getID();
    }
    return result;
}
