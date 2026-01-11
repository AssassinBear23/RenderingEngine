#pragma once
#include <core/camera.h>
#include <core/rendering/frameBuffer.h>
#include <core/rendering/postProcessing/postProcessingManager.h>
#include <core/rendering/shader.h>
#include <editor/inputManager.h>
#include <editor/panel.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <memory>
#include <vector>

namespace core { class Camera; }

namespace editor
{
    class Panel; // fwd

    class Editor {
    public:
        Editor() = default;
        ~Editor();

        bool init(const char* glsl_version = "#version 430");
        void shutdown();

        void run();  // Main loop now lives here

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
        core::FrameBuffer* GetFrameBuffer() const;
        int    getViewportWidth() const;
        int    getViewportHeight() const;
        bool   viewportFocused() const;

        GLFWwindow* GetWindow() const { return m_window; }

        static EditorContext editorCtx;

    private:
        void drawMainMenu();
        void drawDockspace();
        void renderScene(float deltaTime);

        // Scene management
        void registerDefaultScenes();
        void loadDefaultScene();
        bool tryLoadSavedScene();

        bool m_isRunning = false;

        GLFWwindow* m_window = nullptr;
        bool m_initialized = false;
        ImGuiID m_dockspaceId = 0;

        std::vector<std::unique_ptr<Panel>> m_panels;

        float m_windowRoundingValue = 0.0f;
        float m_windowBorderSize = 0.0f;
        ImVec2 m_windowPadding = ImVec2(0.0f, 0.0f);

        // Pointers to special panels we want to expose
        class ViewportPanel* m_viewport = nullptr;

        // Editor-owned resources
        std::shared_ptr<core::postProcessing::PostProcessingManager> m_postProcessingManager;
        std::unique_ptr<InputManager> m_inputManager;
        std::unique_ptr<core::Camera> m_editorCamera;
        std::unique_ptr<core::FrameBuffer> m_sceneRenderBuffer;
        GLuint m_uboLights = 0;

        // Shaders for default scenes
        std::unique_ptr<core::Shader> m_modelShader;
        std::unique_ptr<core::Shader> m_textureShader;
        std::unique_ptr<core::Shader> m_lightBulbShader;
        std::unique_ptr<core::Shader> m_litSurfaceShader;

        friend class ViewportPanel;
    };
}