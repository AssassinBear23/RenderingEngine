#pragma once
#include <memory>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "Panel.h"

namespace editor
{
    class Panel; // fwd

    class Editor {
    public:
        Editor() = default;
        ~Editor();

        void init(GLFWwindow* window, const char* glsl_version = "#version 400");
        void shutdown();

        void beginFrame();   // ImGui new frame
        void draw();         // Dockspace + main menu + panels
        void endFrame();     // Render draw data

        // Panel management
        template<class T, class... Args>
        T& addPanel(Args&&... args) {
            m_panels.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
            return static_cast<T&>(*m_panels.back());
        }
        const std::vector<std::unique_ptr<Panel>>& panels() const { return m_panels; }


        // Viewport render target (owned by ViewportPanel; Editor proxies these)
        GLuint framebuffer() const;  // 0 if not ready
        int    getViewportWidth() const;
        int    getViewportHeight() const;
        bool   viewportFocused() const;

        friend class ViewportPanel; // to set m_viewport

        static EditorContext editorCtx;
    private:
        void drawMainMenu();
        void drawDockspace();

        GLFWwindow* m_window = nullptr;
        bool m_initialized = false;
        ImGuiID m_dockspaceId = 0;

        std::vector<std::unique_ptr<Panel>> m_panels;

        // Pointers to special panels we want to expose
        class ViewportPanel* m_viewport = nullptr;

        friend class ViewportPanel; // to set m_viewport
    };
}