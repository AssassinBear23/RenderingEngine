#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace core {

    /// <summary>
    /// FPS-style camera using Euler angles (degrees) for yaw and pitch.
    /// Maintains an orthonormal basis (forward/right/up) derived from yaw, pitch, and worldUp.
    /// </summary>
    /// <remarks>
    /// - Default yaw = -90 aligns the initial forward with -Z.
    /// - Movement methods translate along the current local basis.
    /// - View matrix uses glm::lookAt(position, position + forward, up).
    /// - Projection uses perspective(FOV 45°, aspect = width/height, near = 0.1, far = 100).
    /// </remarks>
    class Camera {
    public:
        /// <summary>
        /// World-space position of the camera origin.
        /// </summary>
        glm::vec3 position;

        /// <summary>
        /// Reference world-space up vector (e.g., (0,1,0)).
        /// Used when constructing the local basis.
        /// </summary>
        glm::vec3 worldUp;

        /// <summary>
        /// Camera local forward (view) direction (normalized).
        /// Computed from yaw and pitch.
        /// </summary>
        glm::vec3 forward{};

        /// <summary>
        /// Camera local right direction (normalized).
        /// Computed as normalize(cross(forward, worldUp)).
        /// </summary>
        glm::vec3 right{};

        /// <summary>
        /// Camera local up direction (normalized).
        /// Computed as normalize(cross(right, forward)).
        /// </summary>
        glm::vec3 up{};

        /// <summary>
        /// Yaw angle in degrees (rotation around world up). Default -90.
        /// </summary>
        float yaw = -90.0f;

        /// <summary>
        /// Pitch angle in degrees (rotation around camera right). Default 0.
        /// </summary>
        float pitch = 0.0f;

    public:
        /// <summary>
        /// Initializes a new instance and builds the initial basis via UpdateCameraVectors().
        /// </summary>
        /// <param name="pos">: Initial world-space position. </param>
        /// <param name="upDir">: Reference world-space up vector (e.g., (0,1,0)). </param>
        Camera(const glm::vec3& pos, const glm::vec3& upDir);

        /// <summary>
        /// Translates position by forward * speed.
        /// </summary>
        /// <param name="speed">: Distance to move forward (world units). </param>
        void MoveForward(float speed);

        /// <summary>
        /// Translates position by -forward * speed.
        /// </summary>
        /// <param name="speed">: Distance to move backward (world units). </param>
        void MoveBackward(float speed);

        /// <summary>
        /// Translates position by -right * speed.
        /// </summary>
        /// <param name="speed">: Distance to move left (world units). </param>
        void MoveLeft(float speed);

        /// <summary>
        /// Translates position by right * speed.
        /// </summary>
        /// <param name="speed">: Distance to move right (world units). </param>
        void MoveRight(float speed);

        /// <summary>
        /// Translates position by up * speed.
        /// </summary>
        /// <param name="speed">: Distance to move up (world units). </param>
        void MoveUp(float speed);

        /// <summary>
        /// Translates position by -up * speed.
        /// </summary>
        /// <param name="speed">: Distance to move down (world units). </param>
        void MoveDown(float speed);

        /// <summary>
        /// Adjusts yaw and pitch by delta * 0.1 per axis, clamps pitch to [-89, 89], then updates the basis.
        /// </summary>
        /// <param name="delta">: Rotation delta (degrees per unit before 0.1 scaling). delta.x -> yaw, delta.y -> pitch. </param>
        void PivotRotate(glm::vec2 delta);

        /// <summary>
        /// Returns glm::lookAt(position, position + forward, up).
        /// </summary>
        /// <returns>A 4x4 view matrix.</returns>
        glm::mat4 GetViewMatrix() const;

        /// <summary>
        /// Returns a perspective matrix with FOV 45°, aspect = width/height, near = 0.1, far = 100.
        /// </summary>
        /// <param name="width">: Viewport width used for aspect ratio. </param>
        /// <param name="height">: Viewport height used for aspect ratio. </param>
        /// <returns>A 4x4 projection matrix.</returns>
        glm::mat4 GetProjectionMatrix(const float width, const float height) const;

    private:
        /// <summary>
        /// Rebuilds basis vectors from yaw/pitch (degrees):
        /// </summary>
        void UpdateCameraVectors();
    };
}