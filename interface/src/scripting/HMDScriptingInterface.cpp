﻿//
//  HMDScriptingInterface.cpp
//  interface/src/scripting
//
//  Created by Thijs Wenker on 1/12/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "HMDScriptingInterface.h"

#include <QtScript/QScriptContext>

#include "display-plugins/DisplayPlugin.h"
#include <avatar/AvatarManager.h>
#include "Application.h"

HMDScriptingInterface::HMDScriptingInterface() {
}

glm::vec3 HMDScriptingInterface::calculateRayUICollisionPoint(const glm::vec3& position, const glm::vec3& direction) const {
    glm::vec3 result;
    qApp->getApplicationCompositor().calculateRayUICollisionPoint(position, direction, result);
    return result;
}

glm::vec2 HMDScriptingInterface::overlayFromWorldPoint(const glm::vec3& position) const {
    return qApp->getApplicationCompositor().overlayFromSphereSurface(position);
}

glm::vec3 HMDScriptingInterface::worldPointFromOverlay(const glm::vec2& overlay) const {
    return qApp->getApplicationCompositor().sphereSurfaceFromOverlay(overlay);
}

glm::vec2 HMDScriptingInterface::sphericalToOverlay(const glm::vec2 & position) const {
    return qApp->getApplicationCompositor().sphericalToOverlay(position);
}

glm::vec2 HMDScriptingInterface::overlayToSpherical(const glm::vec2 & position) const {
    return qApp->getApplicationCompositor().overlayToSpherical(position);
}

QScriptValue HMDScriptingInterface::getHUDLookAtPosition2D(QScriptContext* context, QScriptEngine* engine) {
    glm::vec3 hudIntersection;
    auto instance = DependencyManager::get<HMDScriptingInterface>();
    if (instance->getHUDLookAtPosition3D(hudIntersection)) {
        glm::vec2 overlayPos = qApp->getApplicationCompositor().overlayFromSphereSurface(hudIntersection);
        return qScriptValueFromValue<glm::vec2>(engine, overlayPos);
    }
    return QScriptValue::NullValue;
}

QScriptValue HMDScriptingInterface::getHUDLookAtPosition3D(QScriptContext* context, QScriptEngine* engine) {
    glm::vec3 result;
    auto instance = DependencyManager::get<HMDScriptingInterface>();
    if (instance->getHUDLookAtPosition3D(result)) {
        return qScriptValueFromValue<glm::vec3>(engine, result);
    }
    return QScriptValue::NullValue;
}

bool HMDScriptingInterface::getHUDLookAtPosition3D(glm::vec3& result) const {
    Camera* camera = qApp->getCamera();
    glm::vec3 position = camera->getPosition();
    glm::quat orientation = camera->getOrientation();

    glm::vec3 direction = orientation * glm::vec3(0.0f, 0.0f, -1.0f);

    const auto& compositor = qApp->getApplicationCompositor();

    return compositor.calculateRayUICollisionPoint(position, direction, result);
}

glm::mat4 HMDScriptingInterface::getWorldHMDMatrix() const {
    MyAvatar* myAvatar = DependencyManager::get<AvatarManager>()->getMyAvatar();
    return myAvatar->getSensorToWorldMatrix() * myAvatar->getHMDSensorMatrix();
}

glm::vec3 HMDScriptingInterface::getPosition() const {
    if (qApp->getActiveDisplayPlugin()->isHmd()) {
        return extractTranslation(getWorldHMDMatrix());
    }
    return glm::vec3();
}

glm::quat HMDScriptingInterface::getOrientation() const {
    if (qApp->getActiveDisplayPlugin()->isHmd()) {
        return glm::normalize(glm::quat_cast(getWorldHMDMatrix()));
    }
    return glm::quat();
}
