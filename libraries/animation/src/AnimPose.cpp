//
//  AnimPose.cpp
//
//  Created by Anthony J. Thibault on 10/14/15.
//  Copyright (c) 2015 High Fidelity, Inc. All rights reserved.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "AnimPose.h"
#include <GLMHelpers.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

const AnimPose AnimPose::identity = AnimPose(glm::vec3(1.0f),
                                             glm::quat(),
                                             glm::vec3(0.0f));

AnimPose::AnimPose(const glm::mat4& mat) {
    scale = extractScale(mat);
    // quat_cast doesn't work so well with scaled matrices, so cancel it out.
    glm::mat4 tmp = glm::scale(mat, 1.0f / scale);
    rot = glm::normalize(glm::quat_cast(tmp));
    trans = extractTranslation(mat);
}

glm::vec3 AnimPose::operator*(const glm::vec3& rhs) const {
    return trans + (rot * (scale * rhs));
}

glm::vec3 AnimPose::xformPoint(const glm::vec3& rhs) const {
    return *this * rhs;
}

// really slow
glm::vec3 AnimPose::xformVector(const glm::vec3& rhs) const {
    glm::vec3 xAxis = rot * glm::vec3(scale.x, 0.0f, 0.0f);
    glm::vec3 yAxis = rot * glm::vec3(0.0f, scale.y, 0.0f);
    glm::vec3 zAxis = rot * glm::vec3(0.0f, 0.0f, scale.z);
    glm::mat3 mat(xAxis, yAxis, zAxis);
    glm::mat3 transInvMat = glm::inverse(glm::transpose(mat));
    return transInvMat * rhs;
}

AnimPose AnimPose::operator*(const AnimPose& rhs) const {
    return AnimPose(static_cast<glm::mat4>(*this) * static_cast<glm::mat4>(rhs));
}

AnimPose AnimPose::inverse() const {
    return AnimPose(glm::inverse(static_cast<glm::mat4>(*this)));
}

// mirror about x-axis without applying negative scale.
AnimPose AnimPose::mirror() const {
    return AnimPose(scale, glm::quat(rot.w, rot.x, -rot.y, -rot.z), glm::vec3(-trans.x, trans.y, trans.z));
}

AnimPose::operator glm::mat4() const {
    glm::vec3 xAxis = rot * glm::vec3(scale.x, 0.0f, 0.0f);
    glm::vec3 yAxis = rot * glm::vec3(0.0f, scale.y, 0.0f);
    glm::vec3 zAxis = rot * glm::vec3(0.0f, 0.0f, scale.z);
    return glm::mat4(glm::vec4(xAxis, 0.0f), glm::vec4(yAxis, 0.0f),
                     glm::vec4(zAxis, 0.0f), glm::vec4(trans, 1.0f));
}
