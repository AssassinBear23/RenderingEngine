#include "core/camera.h"
#include "inputManager.h"
#include <imgui_impl_glfw.h>

namespace editor
{
    InputManager* InputManager::s_instance = nullptr;

    void InputManager::Initialize(GLFWwindow* window)
    {
        s_instance = this;
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetCursorPosCallback(window, CursorPosCallback);
        glfwSetScrollCallback(window, ScrollCallback);
        glfwSetCharCallback(window, CharCallback);
    }

    void InputManager::ProcessInput(GLFWwindow* window, core::Camera* camera, const float deltaTime)
    {
        if (!camera) return;

        // Apply rotation
        if (m_rotating && (m_mouseDeltaX != 0.0 || m_mouseDeltaY != 0.0))
        {
            camera->PivotRotate(glm::vec2(
                static_cast<float>(m_mouseDeltaX),
                static_cast<float>(m_mouseDeltaY)
            ));
            m_mouseDeltaX = 0.0;
            m_mouseDeltaY = 0.0;
        }

        // Then apply movement with updated vectors
        float speed_multiplier = m_keyMap[GLFW_KEY_LEFT_SHIFT] ? 5.0f : 1.0f;

        if (m_keyMap[GLFW_KEY_W]) camera->MoveForward(speed_multiplier * deltaTime);
        if (m_keyMap[GLFW_KEY_S]) camera->MoveBackward(speed_multiplier * deltaTime);
        if (m_keyMap[GLFW_KEY_A]) camera->MoveLeft(speed_multiplier * deltaTime);
        if (m_keyMap[GLFW_KEY_D]) camera->MoveRight(speed_multiplier * deltaTime);
        if (m_keyMap[GLFW_KEY_Q]) camera->MoveDown(speed_multiplier * deltaTime);
        if (m_keyMap[GLFW_KEY_E]) camera->MoveUp(speed_multiplier * deltaTime);
    }

    bool InputManager::IsKeyPressed(int key) const
    {
        return false;
    }

    bool InputManager::IsMouseButtonPressed(int button) const
    {
        return false;
    }

    void InputManager::KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
    {
        // Let ImGui process the key event first
        ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

        if (!s_instance) return;

        if (action == GLFW_PRESS)
            s_instance->m_keyMap[key] = true;
        else if (action == GLFW_RELEASE)
            s_instance->m_keyMap[key] = false;
    }

    void InputManager::MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
    {
        // Forward to ImGui first
        ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

        if (!s_instance) return;

        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                s_instance->m_rotating = true;
                s_instance->m_firstMouse = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            else if (action == GLFW_RELEASE)
            {
                s_instance->m_rotating = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
        }

        if (action == GLFW_PRESS)
            s_instance->m_keyMap[button] = true;
        else if (action == GLFW_RELEASE)
            s_instance->m_keyMap[button] = false;
    }

    void InputManager::CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
    {
        // Forward to Imgui First
        ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

        if (!s_instance || !s_instance->m_rotating) return;

        if (s_instance->m_firstMouse)
        {
            s_instance->m_lastX = xpos;
            s_instance->m_lastY = ypos;
            s_instance->m_firstMouse = false;
            return;
        }

        // Accumulate mouse delta
        s_instance->m_mouseDeltaX += xpos - s_instance->m_lastX;
        s_instance->m_mouseDeltaY += s_instance->m_lastY - ypos; // Invert Y for typical camera control

        s_instance->m_lastX = xpos;
        s_instance->m_lastY = ypos;
    }

    void InputManager::ScrollCallback(GLFWwindow* window, double xoff, double yoff)
    {
        ImGui_ImplGlfw_ScrollCallback(window, xoff, yoff);
    }

    void InputManager::CharCallback(GLFWwindow* window, unsigned int c)
    {
        ImGui_ImplGlfw_CharCallback(window, c);
    }
} // namespace editor