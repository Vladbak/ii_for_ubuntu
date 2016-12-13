#pragma once

#include <GLMHelpers.h>

class Camera {
protected:
    float fov { 60.0f };
    float znear { DEFAULT_NEAR_CLIP }, zfar { DEFAULT_FAR_CLIP };
    float aspect { 1.0f };

    void updateViewMatrix() {
        matrices.view = glm::inverse(glm::translate(glm::mat4(), position) * glm::mat4_cast(getOrientation()));
    }

public:
    const glm::quat& getOrientation() const {
        return orientation;
    }

    glm::quat orientation;
    glm::vec3 position;
    float rotationSpeed { 1.0f };
    float movementSpeed { 1.0f };

    struct Matrices {
        glm::mat4 perspective;
        glm::mat4 view;
    } matrices;
    enum Key {
        RIGHT,
        LEFT,
        UP,
        DOWN,
        BACK,
        FORWARD,
        MLEFT,
        MMIDDLE,
        MRIGHT,
        KEYS_SIZE,
        INVALID = -1,
    };

    std::bitset<KEYS_SIZE> keys;

    Camera() {
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    }

    bool moving() {
        return keys.any();
    }

    void setFieldOfView(float fov) {
        this->fov = fov;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    }

    void setAspectRatio(const glm::vec2& size) {
        setAspectRatio(size.x / size.y);
    }

    void setAspectRatio(float aspect) {
        this->aspect = aspect;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    }

    void setPerspective(float fov, const glm::vec2& size, float znear = 0.1f, float zfar = 512.0f) {
        setPerspective(fov, size.x / size.y, znear, zfar);
    }

    void setPerspective(float fov, float aspect, float znear = 0.1f, float zfar = 512.0f) {
        this->aspect = aspect;
        this->fov = fov;
        this->znear = znear;
        this->zfar = zfar;
        matrices.perspective = glm::perspective(glm::radians(fov), aspect, znear, zfar);
    };

    void rotate(const float delta) {
        orientation = glm::angleAxis(delta, Vectors::UP) * orientation;
        updateViewMatrix();
    }

    void rotate(const glm::vec2& delta) {

        // create orientation vectors
        auto lookat = orientation * Vectors::UNIT_NEG_Z;
        auto forward = glm::normalize(vec3(lookat.x, 0, lookat.z));
        auto side = glm::cross(Vectors::UP, forward);

        // rotate camera with quaternions created from axis and angle
        orientation = glm::angleAxis(delta.x, Vectors::UP) * orientation;
        orientation = glm::angleAxis(-delta.y, side) * orientation;
        updateViewMatrix();
    }

    void setRotation(const glm::quat& rotation) {
        orientation = rotation;
        updateViewMatrix();
    }

    void setPosition(const glm::vec3& position) {
        this->position = position;
        updateViewMatrix();
    }

    // Translate in the Z axis of the camera
    void dolly(float delta) {
        auto direction = glm::vec3(0, 0, delta);
        translate(direction);
    }

    // Translate in the XY plane of the camera
    void translate(const glm::vec2& delta) {
        auto move = glm::vec3(delta.x, delta.y, 0);
        translate(move);
    }

    void translate(const glm::vec3& delta) {
        position += getOrientation() * delta;
        updateViewMatrix();
    }

    void update(float deltaTime) {
        if (moving()) {
            glm::vec3 camFront = getOrientation() * Vectors::FRONT;
            glm::vec3 camRight = getOrientation() * Vectors::RIGHT;
            glm::vec3 camUp = getOrientation() * Vectors::UP;
            float moveSpeed = deltaTime * movementSpeed;

            if (keys[FORWARD]) {
                position += camFront * moveSpeed;
            }
            if (keys[BACK]) {
                position -= camFront * moveSpeed;
            }
            if (keys[LEFT]) {
                position -= camRight * moveSpeed;
            }
            if (keys[RIGHT]) {
                position += camRight * moveSpeed;
            }
            if (keys[UP]) {
                position += camUp * moveSpeed;
            }
            if (keys[DOWN]) {
                position -= camUp * moveSpeed;
            }
            updateViewMatrix();
        }
    }
};
