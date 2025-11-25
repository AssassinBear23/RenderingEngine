#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <memory>

namespace core {
    class Camera;
}

namespace editor {

    /// <summary>
    /// Manages all editor input handling including keyboard, mouse, and camera controls.
    /// This is specific to the editor and won't be included in future builds.
    /// </summary>
    class InputManager {
    public:
        InputManager() = default;
        ~InputManager() = default;

        /// <summary>
        /// Initialize the input manager and register GLFW callbacks.
        /// </summary>
        /// <param name="window">The GLFW window to attach callbacks to.</param>
        void Initialize(GLFWwindow* window);

        /// <summary>
        /// Process accumulated input and update the editor camera.
        /// Should be called each frame when the viewport is focused.
        /// </summary>
        /// <param name="window">The GLFW window.</param>
        /// <param name="camera">The editor camera to control.</param>
        /// <param name="deltaTime">Time elapsed since last frame.</param>
        void ProcessInput(GLFWwindow* window, core::Camera* camera, float deltaTime);

        /// <summary>
        /// Check if a specific key is currently pressed.
        /// </summary>
        /// <param name="key">GLFW key code.</param>
        /// <returns>True if the key is pressed.</returns>
        bool IsKeyPressed(int key) const;

        /// <summary>
        /// Check if a specific mouse button is currently pressed.
        /// </summary>
        /// <param name="button">GLFW mouse button code.</param>
        /// <returns>True if the button is pressed.</returns>
        bool IsMouseButtonPressed(int button) const;

        /// <summary>
        /// Check if the camera is currently rotating (right mouse button held).
        /// </summary>
        /// <returns>True if rotating.</returns>
        bool IsRotating() const { return m_rotating; }

    private:
        // GLFW callback functions (static to match C callback signature)
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos);
        static void ScrollCallback(GLFWwindow* window, double xoff, double yoff);
        static void CharCallback(GLFWwindow* window, unsigned int c);

        // Internal state
        std::unordered_map<int, bool> m_keyMap;
        bool m_rotating = false;
        bool m_firstMouse = true;
        double m_lastX = 0.0;
        double m_lastY = 0.0;
        double m_mouseDeltaX = 0.0;
        double m_mouseDeltaY = 0.0;

        // Static instance for callbacks
        static InputManager* s_instance;
    };

} // namespace editor