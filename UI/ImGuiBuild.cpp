#include "imGuiBuild.h"

namespace UI
{
	Gui::~Gui() { if (m_initialized) Shutdown(); }

	void Gui::Init(GLFWwindow* window, const char* glsl_version)
	{
		m_window = window;

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		// io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // enable later if you want multi-viewport

		ImGui::StyleColorsDark();

		// Backend init
		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		m_initialized = true;
	}

	void Gui::Shutdown()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		m_initialized = false;
	}

	void Gui::BeginFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Gui::Draw()
	{
		// Fullscreen invisible window with a DockSpace
		ImGuiWindowFlags window_flags =
			ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		ImGui::Begin("DockSpaceHost", nullptr, window_flags);
		ImGui::PopStyleVar(2);

		ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
		// If you don’t want pass-through clicks, remove PassthruCentralNode.
		m_dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(m_dockspace_id, ImVec2(0, 0), dock_flags);

		// Top menu bar
		drawMainMenu();

		// Panels
		drawHeirarchyPanel();

		// Optional: ImGui demo window toggle
		if (showDemo) ImGui::ShowDemoWindow(&showDemo);

		ImGui::End();
	}

	void Gui::EndFrame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
	void Gui::drawMainMenu()
	{
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) { /* TODO */ }
				if (ImGui::MenuItem("Open...", "Ctrl+O")) { /* TODO */ }
				if (ImGui::MenuItem("Save", "Ctrl+S")) { /* TODO */ }
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) { /* TODO */ }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				ImGui::MenuItem("ImGui Demo", nullptr, &showDemo);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Settings")) {
				ImGui::SliderFloat("Rotation speed (deg/s)", &rotation_speed_deg_per_s, 0.0f, 720.0f);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}
	void Gui::drawHeirarchyPanel()
	{
		// Placeholder hierarchy until your GameObject graph is ready
		ImGui::Begin("Hierarchy");
		if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
			// Examples – replace with your actual GameObjects
			ImGui::BulletText("Camera");
			if (ImGui::TreeNode("Suzanne")) {
				ImGui::BulletText("MeshRenderer");
				ImGui::BulletText("Transform");
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Quad")) {
				ImGui::BulletText("MeshRenderer");
				ImGui::BulletText("Transform");
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
		ImGui::End();
	}
}