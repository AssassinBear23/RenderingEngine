#include "core/sceneManager.h"
#include "Editor.h"
#include "panels/ViewportPanel.h"

namespace editor
{
    // Define the static member
    EditorContext Editor::editorCtx;

    Editor::~Editor() { if (m_initialized) shutdown(); }

    void Editor::init(GLFWwindow* window, const char* glsl_version) {
        assert(window);
        m_window = window;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        // io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // optional

        ImGui::StyleColorsDark();
        // Optional global style flattening if you want tight edges:
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(0, 0);
        style.FramePadding = ImVec2(4, 4);

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        m_initialized = true;
    }

    void Editor::shutdown() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_initialized = false;
    }

    void Editor::beginFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Editor::drawDockspace() {
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_windowBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_windowBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_windowPadding);
        ImGui::Begin("DockSpaceHost", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        m_dockspaceId = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(m_dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_None);
    }

    void Editor::drawMainMenu() {
        if (!ImGui::BeginMainMenuBar()) return;

        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) { glfwSetWindowShouldClose(m_window, true); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::BeginMenu("Scene"))
            {
                if (editorCtx.sceneManager)
                {
                    auto sceneNames = editorCtx.sceneManager->GetSceneNames();
                    for (const auto& sceneName : sceneNames)
                    {
                        if (ImGui::MenuItem(sceneName.c_str()))
                        {
                            editorCtx.sceneManager->LoadScene(sceneName);
                        }
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Extra Options"))
            {
                ImGui::DragFloat("Font Scale", &ImGui::GetIO().FontGlobalScale, 0.01f, 0.5f, 3.0f);
                ImGui::DragFloat("Window Rounding", &m_windowRoundingValue, 0.1f, 0.0f, 12.0f);
                ImGui::DragFloat("Window Border Size", &m_windowBorderSize, 0.1f, 0.0f, 5.0f);
                ImGui::DragFloat2("Window Padding", &m_windowPadding[0], 0.1f, 0.0f, 20.0f);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows")) {
            for (auto& p : m_panels) {
                ImGui::Checkbox(p->name(), &p->isVisible);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    void Editor::draw() {
        drawDockspace();
        drawMainMenu();

        for (auto& p : m_panels)
            if (p->isVisible)
                p->draw(editorCtx);

        ImGui::End(); // DockSpaceHost
    }

    void Editor::endFrame() {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    GLuint Editor::framebuffer() const { return m_viewport ? m_viewport->framebuffer() : 0; }
    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    int    Editor::getViewportWidth() const { return m_viewport ? m_viewport->GetWidth() : 0; }
    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    int    Editor::getViewportHeight() const { return m_viewport ? m_viewport->GetHeight() : 0; }
    /// <summary>
    /// 
    /// </summary>
    /// <returns></returns>
    bool   Editor::viewportFocused() const { return m_viewport ? m_viewport->isFocused() : false; }
}