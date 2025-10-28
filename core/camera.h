//
// Created by beer on 13/10/2025.
//

#ifndef CAMERA_H
#define CAMERA_H

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {
    class Camera {
    public:
        glm::vec3 position;
        glm::vec3 worldUp;
        glm::vec3 forward{};
        glm::vec3 right{};
        glm::vec3 up{};
        float yaw = -90.0f;
        float pitch = 0.0f;

    public:
        Camera(const glm::vec3& pos, const glm::vec3& upDir);

        void MoveForward(float speed);
        void MoveBackward(float speed);
        void MoveLeft(float speed);
        void MoveRight(float speed);
        void MoveUp(float speed);
        void MoveDown(float speed);
        void PivotRotate(glm::vec2 delta);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix(float width, float height) const;

    private:
        void UpdateCameraVectors();
    };
}

#endif //RAWENGINE_CAMERA_H