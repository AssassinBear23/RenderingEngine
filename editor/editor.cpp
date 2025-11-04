#include "Editor.h"
#include "panels/ViewportPanel.h"
#include <cassert>

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
     style.FramePadding  = ImVec2(4, 4);

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

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("DockSpaceHost", nullptr, window_flags);
    ImGui::PopStyleVar(3);

    m_dockspaceId = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(m_dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_None);
}

void Editor::drawMainMenu() {
    if (!ImGui::BeginMainMenuBar()) return;

    if (ImGui::BeginMenu("File")) {
        if (ImGui::MenuItem("Exit")) { /* hook up as needed */ }
        ImGui::EndMenu();
    }
     if (ImGui::BeginMenu("Settings")) {
         ImGui::SliderFloat("Suzanne Rotation Speed", &rotation_speed_deg_per_s, 0.0f, 720.0f);
         ImGui::EndMenu();
	 }
    if (ImGui::BeginMenu("Windows")) {
        for (auto& p : m_panels) {
            ImGui::Checkbox(p->name(), p->visiblePtr());
        }
        ImGui::MenuItem("ImGui Demo", nullptr, &m_ctx.showDemo);
        ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
}

void Editor::draw() {
    drawDockspace();
    drawMainMenu();

    for (auto& p : m_panels)
        if (*p->visiblePtr())
            p->draw(m_ctx);

    if (m_ctx.showDemo) ImGui::ShowDemoWindow(&m_ctx.showDemo);

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
int    Editor::getViewportWidth() const { return m_viewport ? m_viewport->width() : 0; }
/// <summary>
/// 
/// </summary>
/// <returns></returns>
int    Editor::getViewportHeight() const { return m_viewport ? m_viewport->height() : 0; }
/// <summary>
/// 
/// </summary>
/// <returns></returns>
bool   Editor::viewportFocused() const { return m_viewport ? m_viewport->isFocused() : false; }
