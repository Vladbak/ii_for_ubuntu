//
//  Cube3DOverlay.cpp
//  interface/src/ui/overlays
//
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

// include this before QGLWidget, which includes an earlier version of OpenGL
#include "Cube3DOverlay.h"

#include <SharedUtil.h>
#include <StreamUtils.h>
#include <GeometryCache.h>
#include <DependencyManager.h>

QString const Cube3DOverlay::TYPE = "cube";

Cube3DOverlay::Cube3DOverlay(const Cube3DOverlay* cube3DOverlay) :
    Volume3DOverlay(cube3DOverlay)
{
}

void Cube3DOverlay::render(RenderArgs* args) {
    if (!_visible) {
        return; // do nothing if we're not visible
    }

    float alpha = getAlpha();
    xColor color = getColor();
    const float MAX_COLOR = 255.0f;
    glm::vec4 cubeColor(color.red / MAX_COLOR, color.green / MAX_COLOR, color.blue / MAX_COLOR, alpha);

    // TODO: handle registration point??
    glm::vec3 position = getPosition();
    glm::vec3 dimensions = getDimensions();
    glm::quat rotation = getRotation();

    auto batch = args->_batch;

    if (batch) {
        Transform transform;
        transform.setTranslation(position);
        transform.setRotation(rotation);

        auto geometryCache = DependencyManager::get<GeometryCache>();
        auto pipeline = args->_pipeline;
        if (!pipeline) {
            pipeline = geometryCache->getShapePipeline();
        }

        if (_isSolid) {
            transform.setScale(dimensions);
            batch->setModelTransform(transform);
            geometryCache->renderSolidCubeInstance(*batch, cubeColor, pipeline);
        } else {

            if (getIsDashedLine()) {
                transform.setScale(1.0f);
                batch->setModelTransform(transform);

                glm::vec3 halfDimensions = dimensions / 2.0f;
                glm::vec3 bottomLeftNear(-halfDimensions.x, -halfDimensions.y, -halfDimensions.z);
                glm::vec3 bottomRightNear(halfDimensions.x, -halfDimensions.y, -halfDimensions.z);
                glm::vec3 topLeftNear(-halfDimensions.x, halfDimensions.y, -halfDimensions.z);
                glm::vec3 topRightNear(halfDimensions.x, halfDimensions.y, -halfDimensions.z);

                glm::vec3 bottomLeftFar(-halfDimensions.x, -halfDimensions.y, halfDimensions.z);
                glm::vec3 bottomRightFar(halfDimensions.x, -halfDimensions.y, halfDimensions.z);
                glm::vec3 topLeftFar(-halfDimensions.x, halfDimensions.y, halfDimensions.z);
                glm::vec3 topRightFar(halfDimensions.x, halfDimensions.y, halfDimensions.z);

                geometryCache->renderDashedLine(*batch, bottomLeftNear, bottomRightNear, cubeColor);
                geometryCache->renderDashedLine(*batch, bottomRightNear, bottomRightFar, cubeColor);
                geometryCache->renderDashedLine(*batch, bottomRightFar, bottomLeftFar, cubeColor);
                geometryCache->renderDashedLine(*batch, bottomLeftFar, bottomLeftNear, cubeColor);

                geometryCache->renderDashedLine(*batch, topLeftNear, topRightNear, cubeColor);
                geometryCache->renderDashedLine(*batch, topRightNear, topRightFar, cubeColor);
                geometryCache->renderDashedLine(*batch, topRightFar, topLeftFar, cubeColor);
                geometryCache->renderDashedLine(*batch, topLeftFar, topLeftNear, cubeColor);

                geometryCache->renderDashedLine(*batch, bottomLeftNear, topLeftNear, cubeColor);
                geometryCache->renderDashedLine(*batch, bottomRightNear, topRightNear, cubeColor);
                geometryCache->renderDashedLine(*batch, bottomLeftFar, topLeftFar, cubeColor);
                geometryCache->renderDashedLine(*batch, bottomRightFar, topRightFar, cubeColor);

            } else {
                transform.setScale(dimensions);
                batch->setModelTransform(transform);
                geometryCache->renderWireCubeInstance(*batch, cubeColor, pipeline);
            }
        }
    }
}

const render::ShapeKey Cube3DOverlay::getShapeKey() {
    auto builder = render::ShapeKey::Builder();
    if (getAlpha() != 1.0f) {
        builder.withTranslucent();
    }
    return builder.build();
}

Cube3DOverlay* Cube3DOverlay::createClone() const {
    return new Cube3DOverlay(this);
}

void Cube3DOverlay::setProperties(const QVariantMap& properties) {
    Volume3DOverlay::setProperties(properties);

    auto borderSize = properties["borderSize"];

    if (borderSize.isValid()) {
        float value = borderSize.toFloat();
        setBorderSize(value);
    }
}

QVariant Cube3DOverlay::getProperty(const QString& property) {
    if (property == "borderSize") {
        return _borderSize;
    }

    return Volume3DOverlay::getProperty(property);
}
