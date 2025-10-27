//
// Created by beer on 13/10/2025.
//

#include "camera.h"

namespace core {
    Camera::Camera(const glm::vec3& pos, const glm::vec3& upDir)
        : position(pos), worldUp(upDir){
        updateCameraVectors();
    }

    void Camera::moveForward(const float speed) {
        position += forward * speed;
    }

    void Camera::moveBackward(const float speed) {
        position -= forward * speed;
    }

    void Camera::moveLeft(const float speed) {
        position -= right * speed;
    }

    void Camera::moveRight(const float speed) {
        position += right * speed;
    }

    void Camera::moveUp(const float speed) {
        position += up * speed;
    }

    void Camera::moveDown(const float speed) {
        position -= up * speed;
    }

    void Camera::pivotRotate(const glm::vec2 delta) {
        yaw   += delta.x * 0.1f;
        pitch += delta.y * 0.1f;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
        updateCameraVectors();
    }

    glm::mat4 Camera::getViewMatrix() const {
        glm::mat4 const view = glm::lookAt(position, position + forward, up);
        return view;
    }

    glm::mat4 Camera::getProjectionMatrix(const float width, const float height) const { // NOLINT(*-convert-member-functions-to-static)
        glm::mat4 const projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
        return projection;
    }

    void Camera::updateCameraVectors() {
        glm::vec3 dir{};
        dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        dir.y = sin(glm::radians(pitch));
        dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        forward = glm::normalize(dir);
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
    }
} // core