//
//  ApplicationCompositor.cpp
//  interface/src/ui/overlays
//
//  Created by Benjamin Arnold on 5/27/14.
//  Copyright 2014 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

#include "ApplicationCompositor.h"

#include <memory>

#include <QPropertyAnimation>

#include <glm/gtc/type_ptr.hpp>

#include <display-plugins/DisplayPlugin.h>
#include <avatar/AvatarManager.h>
#include <gpu/GLBackend.h>
#include <NumericalConstants.h>

#include "CursorManager.h"
#include "Tooltip.h"

#include "Application.h"
#include <controllers/InputDevice.h>


// Used to animate the magnification windows

static const quint64 MSECS_TO_USECS = 1000ULL;
static const quint64 TOOLTIP_DELAY = 500 * MSECS_TO_USECS;

static const float reticleSize = TWO_PI / 100.0f;

static const float CURSOR_PIXEL_SIZE = 32.0f;
static const float MOUSE_PITCH_RANGE = 1.0f * PI;
static const float MOUSE_YAW_RANGE = 0.5f * TWO_PI;
static const glm::vec2 MOUSE_RANGE(MOUSE_YAW_RANGE, MOUSE_PITCH_RANGE);

static gpu::BufferPointer _hemiVertices;
static gpu::BufferPointer _hemiIndices;
static int _hemiIndexCount{ 0 };
EntityItemID ApplicationCompositor::_noItemId;
static QString _tooltipId;

// Return a point's cartesian coordinates on a sphere from pitch and yaw
glm::vec3 getPoint(float yaw, float pitch) {
    return glm::vec3(glm::cos(-pitch) * (-glm::sin(yaw)),
                     glm::sin(-pitch),
                     glm::cos(-pitch) * (-glm::cos(yaw)));
}

//Checks if the given ray intersects the sphere at the origin. result will store a multiplier that should
//be multiplied by dir and added to origin to get the location of the collision
bool raySphereIntersect(const glm::vec3 &dir, const glm::vec3 &origin, float r, float* result)
{
    //Source: http://wiki.cgsociety.org/index.php/Ray_Sphere_Intersection

    //Compute A, B and C coefficients
    float a = glm::dot(dir, dir);
    float b = 2 * glm::dot(dir, origin);
    float c = glm::dot(origin, origin) - (r * r);

    //Find discriminant
    float disc = b * b - 4 * a * c;

    // if discriminant is negative there are no real roots, so return
    // false as ray misses sphere
    if (disc < 0) {
        return false;
    }

    // compute q as described above
    float distSqrt = sqrtf(disc);
    float q;
    if (b < 0) {
        q = (-b - distSqrt) / 2.0f;
    } else {
        q = (-b + distSqrt) / 2.0f;
    }

    // compute t0 and t1
    float t0 = q / a;
    float t1 = c / q;

    // make sure t0 is smaller than t1
    if (t0 > t1) {
        // if t0 is bigger than t1 swap them around
        float temp = t0;
        t0 = t1;
        t1 = temp;
    }

    // if t1 is less than zero, the object is in the ray's negative direction
    // and consequently the ray misses the sphere
    if (t1 < 0) {
        return false;
    }

    // if t0 is less than zero, the intersection point is at t1
    if (t0 < 0) {
        *result = t1;
        return true;
    } else { // else the intersection point is at t0
        *result = t0;
        return true;
    }
}

ApplicationCompositor::ApplicationCompositor() :
    _alphaPropertyAnimation(new QPropertyAnimation(this, "alpha"))
{
    auto geometryCache = DependencyManager::get<GeometryCache>();

    _reticleQuad = geometryCache->allocateID();

    auto entityScriptingInterface = DependencyManager::get<EntityScriptingInterface>();
    connect(entityScriptingInterface.data(), &EntityScriptingInterface::hoverEnterEntity, [=](const EntityItemID& entityItemID, const MouseEvent& event) {
        if (_hoverItemId != entityItemID) {
            _hoverItemId = entityItemID;
            _hoverItemEnterUsecs = usecTimestampNow();
            auto properties = entityScriptingInterface->getEntityProperties(_hoverItemId);

            // check the format of this href string before we parse it
            QString hrefString = properties.getHref();

            auto cursor = Cursor::Manager::instance().getCursor();
            if (!hrefString.isEmpty()) {
                if (!hrefString.startsWith("hifi:")) {
                    hrefString.prepend("hifi://");
                }

                // parse out a QUrl from the hrefString
                QUrl href = QUrl(hrefString);

                _hoverItemTitle = href.host();
                _hoverItemDescription = properties.getDescription();
                cursor->setIcon(Cursor::Icon::LINK);
            } else {
                _hoverItemTitle.clear();
                _hoverItemDescription.clear();
                cursor->setIcon(Cursor::Icon::DEFAULT);
            }
        }
    });

    connect(entityScriptingInterface.data(), &EntityScriptingInterface::hoverLeaveEntity, [=](const EntityItemID& entityItemID, const MouseEvent& event) {
        if (_hoverItemId == entityItemID) {
            _hoverItemId = _noItemId;

            _hoverItemTitle.clear();
            _hoverItemDescription.clear();

            auto cursor = Cursor::Manager::instance().getCursor();
            cursor->setIcon(Cursor::Icon::DEFAULT);
            if (!_tooltipId.isEmpty()) {
                qDebug() << "Closing tooltip " << _tooltipId;
                Tooltip::closeTip(_tooltipId);
                _tooltipId.clear();
            }
        }
    });

    _alphaPropertyAnimation.reset(new QPropertyAnimation(this, "alpha"));
}

ApplicationCompositor::~ApplicationCompositor() {
}


void ApplicationCompositor::bindCursorTexture(gpu::Batch& batch, uint8_t cursorIndex) {
    auto& cursorManager = Cursor::Manager::instance();
    auto cursor = cursorManager.getCursor(cursorIndex);
    auto iconId = cursor->getIcon();
    if (!_cursors.count(iconId)) {
        auto iconPath = cursorManager.getIconImage(cursor->getIcon());
        _cursors[iconId] = DependencyManager::get<TextureCache>()->
            getImageTexture(iconPath);
    }
    batch.setResourceTexture(0, _cursors[iconId]);
}

// Draws the FBO texture for the screen
void ApplicationCompositor::displayOverlayTexture(RenderArgs* renderArgs) {
    PROFILE_RANGE(__FUNCTION__);

    if (_alpha <= 0.0f) {
        return;
    }

    gpu::FramebufferPointer overlayFramebuffer = qApp->getApplicationOverlay().getOverlayFramebuffer();
    if (!overlayFramebuffer) {
        return;
    }

    updateTooltips();

    //Handle fading and deactivation/activation of UI
    gpu::doInBatch(renderArgs->_context, [&](gpu::Batch& batch) {

        auto geometryCache = DependencyManager::get<GeometryCache>();

        geometryCache->useSimpleDrawPipeline(batch);
        batch.setViewportTransform(renderArgs->_viewport);
        batch.setModelTransform(Transform());
        batch.setViewTransform(Transform());
        batch.setProjectionTransform(mat4());
        batch.setResourceTexture(0, overlayFramebuffer->getRenderBuffer(0));
        geometryCache->renderUnitQuad(batch, vec4(vec3(1), _alpha));

        //draw the mouse pointer
        // Get the mouse coordinates and convert to NDC [-1, 1]
        vec2 canvasSize = qApp->getCanvasSize();
        vec2 mousePosition = toNormalizedDeviceScale(vec2(qApp->getMouse()), canvasSize);
        // Invert the Y axis
        mousePosition.y *= -1.0f;

        Transform model;
        model.setTranslation(vec3(mousePosition, 0));
        vec2 mouseSize = CURSOR_PIXEL_SIZE / canvasSize;
        model.setScale(vec3(mouseSize, 1.0f));
        batch.setModelTransform(model);
        bindCursorTexture(batch);
        geometryCache->renderUnitQuad(batch, vec4(1));
    });
}

// Draws the FBO texture for Oculus rift.
void ApplicationCompositor::displayOverlayTextureHmd(RenderArgs* renderArgs, int eye) {
    PROFILE_RANGE(__FUNCTION__);

    if (_alpha <= 0.0f) {
        return;
    }

    gpu::FramebufferPointer overlayFramebuffer = qApp->getApplicationOverlay().getOverlayFramebuffer();
    if (!overlayFramebuffer) {
        return;
    }

    updateTooltips();

    vec2 canvasSize = qApp->getCanvasSize();
    _textureAspectRatio = aspect(canvasSize);

    auto geometryCache = DependencyManager::get<GeometryCache>();

    gpu::doInBatch(renderArgs->_context, [&](gpu::Batch& batch) {
        geometryCache->useSimpleDrawPipeline(batch);

        batch.setResourceTexture(0, overlayFramebuffer->getRenderBuffer(0));

        mat4 camMat;
        _cameraBaseTransform.getMatrix(camMat);
        auto displayPlugin = qApp->getActiveDisplayPlugin();
        auto headPose = displayPlugin->getHeadPose(qApp->getFrameCount());
        auto eyeToHead = displayPlugin->getEyeToHeadTransform((Eye)eye);
        camMat = (headPose * eyeToHead) * camMat;
        batch.setViewportTransform(renderArgs->_viewport);
        batch.setViewTransform(camMat);
        batch.setProjectionTransform(qApp->getEyeProjection(eye));

    #ifdef DEBUG_OVERLAY
        {
            batch.setModelTransform(glm::translate(mat4(), vec3(0, 0, -2)));
            geometryCache->renderUnitQuad(batch, glm::vec4(1));
        }
    #else
        {
            batch.setModelTransform(_modelTransform);
            drawSphereSection(batch);
        }
    #endif


        vec3 reticleScale = vec3(Cursor::Manager::instance().getScale() * reticleSize);

        bindCursorTexture(batch);

        //Mouse Pointer
        auto controllerScriptingInterface = DependencyManager::get<controller::ScriptingInterface>();
        bool reticleVisible = controllerScriptingInterface->getReticleVisible();
        if (reticleVisible) {
            glm::mat4 overlayXfm;
            _modelTransform.getMatrix(overlayXfm);

            glm::vec2 projection = screenToSpherical(qApp->getTrueMouse());

            float cursorDepth = controllerScriptingInterface->getReticleDepth();
            mat4 pointerXfm = glm::scale(mat4(), vec3(cursorDepth)) * glm::mat4_cast(quat(vec3(-projection.y, projection.x, 0.0f))) * glm::translate(mat4(), vec3(0, 0, -1));
            mat4 reticleXfm = overlayXfm * pointerXfm;
            reticleXfm = glm::scale(reticleXfm, reticleScale);
            batch.setModelTransform(reticleXfm);
            geometryCache->renderUnitQuad(batch, glm::vec4(1), _reticleQuad);
        }
    });
}


// FIXME - this probably is hella buggy and probably doesn't work correctly
// we should kill it asap.
void ApplicationCompositor::computeHmdPickRay(glm::vec2 cursorPos, glm::vec3& origin, glm::vec3& direction) const {
    const glm::vec2 projection = overlayToSpherical(cursorPos);
    // The overlay space orientation of the mouse coordinates
    const glm::quat cursorOrientation(glm::vec3(-projection.y, projection.x, 0.0f));

    // The orientation and position of the HEAD, not the overlay
    glm::vec3 worldSpaceHeadPosition = qApp->getCamera()->getPosition();
    glm::quat worldSpaceOrientation = qApp->getCamera()->getOrientation();

    auto headPose = qApp->getHMDSensorPose();
    auto headOrientation = glm::quat_cast(headPose);
    auto headTranslation = extractTranslation(headPose);

    auto overlayOrientation = worldSpaceOrientation * glm::inverse(headOrientation);
    auto overlayPosition = worldSpaceHeadPosition - (overlayOrientation * headTranslation);
    if (Menu::getInstance()->isOptionChecked(MenuOption::StandingHMDSensorMode)) {
        overlayPosition = _modelTransform.getTranslation();
        overlayOrientation = _modelTransform.getRotation();
    }

    // Intersection in world space
    glm::vec3 worldSpaceIntersection = ((overlayOrientation * (cursorOrientation * Vectors::FRONT)) * _oculusUIRadius) + overlayPosition;

    origin = worldSpaceHeadPosition;
    direction = glm::normalize(worldSpaceIntersection - worldSpaceHeadPosition);
}

//Finds the collision point of a world space ray
bool ApplicationCompositor::calculateRayUICollisionPoint(const glm::vec3& position, const glm::vec3& direction, glm::vec3& result) const {

    auto displayPlugin = qApp->getActiveDisplayPlugin();
    auto headPose = displayPlugin->getHeadPose(qApp->getFrameCount());

    auto myCamera = qApp->getCamera();
    mat4 cameraMat = myCamera->getTransform();
    auto UITransform = cameraMat * glm::inverse(headPose);
    auto relativePosition4 = glm::inverse(UITransform) * vec4(position, 1);
    auto relativePosition = vec3(relativePosition4) / relativePosition4.w;
    auto relativeDirection = glm::inverse(glm::quat_cast(UITransform)) * direction;

    float uiRadius = _oculusUIRadius; // * myAvatar->getUniformScale(); // FIXME - how do we want to handle avatar scale

    float instersectionDistance;
    if (raySphereIntersect(relativeDirection, relativePosition, uiRadius, &instersectionDistance)){
        result = position + glm::normalize(direction) * instersectionDistance;
        return true;
    }

    return false;
}

void ApplicationCompositor::buildHemiVertices(
    const float fov, const float aspectRatio, const int slices, const int stacks) {
    static float textureFOV = 0.0f, textureAspectRatio = 1.0f;
    if (textureFOV == fov && textureAspectRatio == aspectRatio) {
        return;
    }

    textureFOV = fov;
    textureAspectRatio = aspectRatio;

    auto geometryCache = DependencyManager::get<GeometryCache>();

    _hemiVertices = std::make_shared<gpu::Buffer>();
    _hemiIndices = std::make_shared<gpu::Buffer>();


    if (fov >= PI) {
        qDebug() << "TexturedHemisphere::buildVBO(): FOV greater or equal than Pi will create issues";
    }

    //UV mapping source: http://www.mvps.org/directx/articles/spheremap.htm

    vec3 pos;
    vec2 uv;
    // Compute vertices positions and texture UV coordinate
    // Create and write to buffer
    for (int i = 0; i < stacks; i++) {
        uv.y = (float)i / (float)(stacks - 1); // First stack is 0.0f, last stack is 1.0f
        // abs(theta) <= fov / 2.0f
        float pitch = -fov * (uv.y - 0.5f);
        for (int j = 0; j < slices; j++) {
            uv.x = (float)j / (float)(slices - 1); // First slice is 0.0f, last slice is 1.0f
            // abs(phi) <= fov * aspectRatio / 2.0f
            float yaw = -fov * aspectRatio * (uv.x - 0.5f);
            pos = getPoint(yaw, pitch);
            static const vec4 color(1);
            _hemiVertices->append(sizeof(pos), (gpu::Byte*)&pos);
            _hemiVertices->append(sizeof(vec2), (gpu::Byte*)&uv);
            _hemiVertices->append(sizeof(vec4), (gpu::Byte*)&color);
        }
    }

    // Compute number of indices needed
    static const int VERTEX_PER_TRANGLE = 3;
    static const int TRIANGLE_PER_RECTANGLE = 2;
    int numberOfRectangles = (slices - 1) * (stacks - 1);
    _hemiIndexCount = numberOfRectangles * TRIANGLE_PER_RECTANGLE * VERTEX_PER_TRANGLE;

    // Compute indices order
    std::vector<GLushort> indices;
    for (int i = 0; i < stacks - 1; i++) {
        for (int j = 0; j < slices - 1; j++) {
            GLushort bottomLeftIndex = i * slices + j;
            GLushort bottomRightIndex = bottomLeftIndex + 1;
            GLushort topLeftIndex = bottomLeftIndex + slices;
            GLushort topRightIndex = topLeftIndex + 1;
            // FIXME make a z-order curve for better vertex cache locality
            indices.push_back(topLeftIndex);
            indices.push_back(bottomLeftIndex);
            indices.push_back(topRightIndex);

            indices.push_back(topRightIndex);
            indices.push_back(bottomLeftIndex);
            indices.push_back(bottomRightIndex);
        }
    }
    _hemiIndices->append(sizeof(GLushort) * indices.size(), (gpu::Byte*)&indices[0]);
}


void ApplicationCompositor::drawSphereSection(gpu::Batch& batch) {
    buildHemiVertices(_textureFov, _textureAspectRatio, 80, 80);
    static const int VERTEX_DATA_SLOT = 0;
    static const int TEXTURE_DATA_SLOT = 1;
    static const int COLOR_DATA_SLOT = 2;
    auto streamFormat = std::make_shared<gpu::Stream::Format>(); // 1 for everyone
    streamFormat->setAttribute(gpu::Stream::POSITION, VERTEX_DATA_SLOT, gpu::Element(gpu::VEC3, gpu::FLOAT, gpu::XYZ), 0);
    streamFormat->setAttribute(gpu::Stream::TEXCOORD, TEXTURE_DATA_SLOT, gpu::Element(gpu::VEC2, gpu::FLOAT, gpu::UV));
    streamFormat->setAttribute(gpu::Stream::COLOR, COLOR_DATA_SLOT, gpu::Element(gpu::VEC4, gpu::FLOAT, gpu::RGBA));
    batch.setInputFormat(streamFormat);

    static const int VERTEX_STRIDE = sizeof(vec3) + sizeof(vec2) + sizeof(vec4);

    if (_prevAlpha != _alpha) {
        // adjust alpha by munging vertex color alpha.
        // FIXME we should probably just use a uniform for this.
        float* floatPtr = reinterpret_cast<float*>(_hemiVertices->editData());
        const auto ALPHA_FLOAT_OFFSET = (sizeof(vec3) + sizeof(vec2) + sizeof(vec3)) / sizeof(float);
        const auto VERTEX_FLOAT_STRIDE = (sizeof(vec3) + sizeof(vec2) + sizeof(vec4)) / sizeof(float);
        const auto NUM_VERTS = _hemiVertices->getSize() / VERTEX_STRIDE;
        for (size_t i = 0; i < NUM_VERTS; i++) {
            floatPtr[i * VERTEX_FLOAT_STRIDE + ALPHA_FLOAT_OFFSET] = _alpha;
        }
    }

    gpu::BufferView posView(_hemiVertices, 0, _hemiVertices->getSize(), VERTEX_STRIDE, streamFormat->getAttributes().at(gpu::Stream::POSITION)._element);
    gpu::BufferView uvView(_hemiVertices, sizeof(vec3), _hemiVertices->getSize(), VERTEX_STRIDE, streamFormat->getAttributes().at(gpu::Stream::TEXCOORD)._element);
    gpu::BufferView colView(_hemiVertices, sizeof(vec3) + sizeof(vec2), _hemiVertices->getSize(), VERTEX_STRIDE, streamFormat->getAttributes().at(gpu::Stream::COLOR)._element);
    batch.setInputBuffer(VERTEX_DATA_SLOT, posView);
    batch.setInputBuffer(TEXTURE_DATA_SLOT, uvView);
    batch.setInputBuffer(COLOR_DATA_SLOT, colView);
    batch.setIndexBuffer(gpu::UINT16, _hemiIndices, 0);
    batch.drawIndexed(gpu::TRIANGLES, _hemiIndexCount);
}

glm::vec2 ApplicationCompositor::screenToSpherical(const glm::vec2& screenPos) {
    auto screenSize = qApp->getCanvasSize();
    glm::vec2 result;
    result.x = -(screenPos.x / screenSize.x - 0.5f);
    result.y = (screenPos.y / screenSize.y - 0.5f);
    result.x *= MOUSE_YAW_RANGE;
    result.y *= MOUSE_PITCH_RANGE;
    return result;
}

glm::vec2 ApplicationCompositor::sphericalToScreen(const glm::vec2& sphericalPos) {
    glm::vec2 result = sphericalPos;
    result.x *= -1.0f;
    result /= MOUSE_RANGE;
    result += 0.5f;
    result *= qApp->getCanvasSize();
    return result;
}

glm::vec2 ApplicationCompositor::sphericalToOverlay(const glm::vec2&  sphericalPos) const {
    glm::vec2 result = sphericalPos;
    result.x *= -1.0f;
    result /= _textureFov;
    result.x /= _textureAspectRatio;
    result += 0.5f;
    result *= qApp->getUiSize();
    return result;
}

glm::vec2 ApplicationCompositor::overlayToSpherical(const glm::vec2&  overlayPos) const {
    glm::vec2 result = overlayPos;
    result /= qApp->getUiSize();
    result -= 0.5f;
    result *= _textureFov;
    result.x *= _textureAspectRatio;
    result.x *= -1.0f;
    return result;
}

glm::vec2 ApplicationCompositor::screenToOverlay(const glm::vec2& screenPos) const {
    return sphericalToOverlay(screenToSpherical(screenPos));
}

glm::vec2 ApplicationCompositor::overlayToScreen(const glm::vec2& overlayPos) const {
    return sphericalToScreen(overlayToSpherical(overlayPos));
}

glm::vec2 ApplicationCompositor::overlayFromSphereSurface(const glm::vec3& sphereSurfacePoint) const {

    auto displayPlugin = qApp->getActiveDisplayPlugin();
    auto headPose = displayPlugin->getHeadPose(qApp->getFrameCount());
    auto myCamera = qApp->getCamera();
    mat4 cameraMat = myCamera->getTransform();
    auto UITransform = cameraMat * glm::inverse(headPose);
    auto relativePosition4 = glm::inverse(UITransform) * vec4(sphereSurfacePoint, 1);
    auto relativePosition = vec3(relativePosition4) / relativePosition4.w;
    auto center = vec3(0); // center of HUD in HUD space
    auto direction = relativePosition - center; // direction to relative position in HUD space

    glm::vec2 polar = glm::vec2(glm::atan(direction.x, -direction.z), glm::asin(direction.y)) * -1.0f;
    auto overlayPos = sphericalToOverlay(polar);
    return overlayPos;
}

void ApplicationCompositor::updateTooltips() {
    if (_hoverItemId != _noItemId) {
        quint64 hoverDuration = usecTimestampNow() - _hoverItemEnterUsecs;
        if (_hoverItemEnterUsecs != UINT64_MAX && !_hoverItemTitle.isEmpty() && hoverDuration > TOOLTIP_DELAY) {
            // TODO Enable and position the tooltip
            _hoverItemEnterUsecs = UINT64_MAX;
            _tooltipId = Tooltip::showTip(_hoverItemTitle, _hoverItemDescription);
        }
    }
}

static const float FADE_DURATION = 500.0f;
void ApplicationCompositor::fadeIn() {
    _fadeInAlpha = true;

    _alphaPropertyAnimation->setDuration(FADE_DURATION);
    _alphaPropertyAnimation->setStartValue(_alpha);
    _alphaPropertyAnimation->setEndValue(1.0f);
    _alphaPropertyAnimation->start();
}
void ApplicationCompositor::fadeOut() {
    _fadeInAlpha = false;

    _alphaPropertyAnimation->setDuration(FADE_DURATION);
    _alphaPropertyAnimation->setStartValue(_alpha);
    _alphaPropertyAnimation->setEndValue(0.0f);
    _alphaPropertyAnimation->start();
}

void ApplicationCompositor::toggle() {
    if (_fadeInAlpha) {
        fadeOut();
    } else {
        fadeIn();
    }
}
