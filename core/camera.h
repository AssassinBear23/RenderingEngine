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

        void moveForward(float speed);
        void moveBackward(float speed);
        void moveLeft(float speed);
        void moveRight(float speed);
        void moveUp(float speed);
        void moveDown(float speed);
        void pivotRotate(glm::vec2 delta);

        glm::mat4 getViewMatrix() const;
        glm::mat4 getProjectionMatrix(float width, float height) const;

    private:
        void updateCameraVectors();
    };
}

#endif //RAWENGINE_CAMERA_H