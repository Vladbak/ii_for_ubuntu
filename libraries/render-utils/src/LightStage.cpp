//
//  LightStage.cpp
//  render-utils/src
//
//  Created by Zach Pomerantz on 1/14/2015.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ViewFrustum.h"

#include "LightStage.h"

LightStage::Shadow::Shadow(model::LightPointer light) : _light{ light}, _frustum{ std::make_shared<ViewFrustum>() } {
    framebuffer = gpu::FramebufferPointer(gpu::Framebuffer::createShadowmap(MAP_SIZE));
    map = framebuffer->getDepthStencilBuffer();
    Schema schema;
    _schemaBuffer = std::make_shared<gpu::Buffer>(sizeof(Schema), (const gpu::Byte*) &schema);
}

void LightStage::Shadow::setKeylightFrustum(const ViewFrustum& viewFrustum, float nearDepth, float farDepth) {
    assert(nearDepth < farDepth);

    // Orient the keylight frustum
    const auto& direction = glm::normalize(_light->getDirection());
    glm::quat orientation;
    if (direction == IDENTITY_UP) {
        orientation = glm::quat(glm::mat3(-IDENTITY_RIGHT, IDENTITY_FRONT, -IDENTITY_UP));
    } else if (direction == -IDENTITY_UP) {
        orientation = glm::quat(glm::mat3(IDENTITY_RIGHT, IDENTITY_FRONT, IDENTITY_UP));
    } else {
        auto side = glm::normalize(glm::cross(direction, IDENTITY_UP));
        auto up = glm::normalize(glm::cross(side, direction));
        orientation = glm::quat_cast(glm::mat3(side, up, -direction));
    }
    _frustum->setOrientation(orientation);

    // Position the keylight frustum
    _frustum->setPosition(viewFrustum.getPosition() - (nearDepth + farDepth)*direction);

    const Transform view{ _frustum->getView()};
    const Transform viewInverse{ view.getInverseMatrix() };

    auto nearCorners = viewFrustum.getCorners(nearDepth);
    auto farCorners = viewFrustum.getCorners(farDepth);

    vec3 min{ viewInverse.transform(nearCorners.bottomLeft) };
    vec3 max{ min };
    // Expand keylight frustum  to fit view frustum
    auto fitFrustum = [&min, &max, &viewInverse](const vec3& viewCorner) {
        const auto corner = viewInverse.transform(viewCorner);

        min.x = glm::min(min.x, corner.x);
        min.y = glm::min(min.y, corner.y);
        min.z = glm::min(min.z, corner.z);

        max.x = glm::max(max.x, corner.x);
        max.y = glm::max(max.y, corner.y);
        max.z = glm::max(max.z, corner.z);
    };
    fitFrustum(nearCorners.bottomRight);
    fitFrustum(nearCorners.topLeft);
    fitFrustum(nearCorners.topRight);
    fitFrustum(farCorners.bottomLeft);
    fitFrustum(farCorners.bottomRight);
    fitFrustum(farCorners.topLeft);
    fitFrustum(farCorners.topRight);

    glm::mat4 ortho = glm::ortho<float>(min.x, max.x, min.y, max.y, -max.z, -min.z);
    _frustum->setProjection(ortho);

    // Calculate the frustum's internal state
    _frustum->calculate();

    // Update the buffer
    _schemaBuffer.edit<Schema>().projection = ortho;
    _schemaBuffer.edit<Schema>().viewInverse = viewInverse.getMatrix();
}

const glm::mat4& LightStage::Shadow::getView() const {
    return _frustum->getView();
}

const glm::mat4& LightStage::Shadow::getProjection() const {
    return _frustum->getProjection();
}

const LightStage::LightPointer LightStage::addLight(model::LightPointer light) {
  //  Shadow stageShadow{light};
    LightPointer stageLight = std::make_shared<Light>(Shadow(light));
    stageLight->light = light;
    lights.push_back(stageLight);
    return stageLight;
}
