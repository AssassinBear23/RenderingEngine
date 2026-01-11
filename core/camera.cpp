#include "camera.h"
#include <cmath>
#include <glm/detail/func_common.inl>
#include <glm/detail/func_geometric.inl>
#include <glm/detail/func_trigonometric.inl>
#include <glm/ext/matrix_clip_space.inl>
#include <glm/ext/matrix_transform.inl>
#include <glm/fwd.hpp>

namespace core {
    Camera::Camera(const glm::vec3& pos, const glm::vec3& upDir)
        : position(pos), worldUp(upDir){
        UpdateCameraVectors();
    }

    void Camera::MoveForward(const float speed) {
        position += forward * speed;
    }

    void Camera::MoveBackward(const float speed) {
        position -= forward * speed;
    }

    void Camera::MoveLeft(const float speed) {
        position -= right * speed;
    }

    void Camera::MoveRight(const float speed) {
        position += right * speed;
    }

    void Camera::MoveUp(const float speed) {
        position += up * speed;
    }

    void Camera::MoveDown(const float speed) {
        position -= up * speed;
    }

    void Camera::PivotRotate(const glm::vec2 delta) {
        yaw   += delta.x * 0.1f;
        pitch += delta.y * 0.1f;
        pitch = glm::clamp(pitch, -89.0f, 89.0f);
        UpdateCameraVectors();
    }

    glm::mat4 Camera::GetViewMatrix() const {
        glm::mat4 const view = glm::lookAt(position, position + forward, up);
        return view;
    }

    glm::mat4 Camera::GetProjectionMatrix(const float width, const float height) const {
        glm::mat4 const projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 100.0f);
        return projection;
    }

    void Camera::UpdateCameraVectors() {
        glm::vec3 dir{};
        dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        dir.y = sin(glm::radians(pitch));
        dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

        forward = glm::normalize(dir);
        right = glm::normalize(glm::cross(forward, worldUp));
        up = glm::normalize(glm::cross(right, forward));
    }
} // core