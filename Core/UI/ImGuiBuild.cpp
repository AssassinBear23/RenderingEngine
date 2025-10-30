#include "ImGuiBuild.h"
#include <iostream>

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
			ImGuiWindowFlags_NoNavFocus;

		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0)); // No interior padding inside the dockspace

		ImGui::Begin("DockSpaceHost", nullptr, window_flags);
		ImGui::PopStyleVar(3);

		ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_None; // Switching to Viewport window, so no more ImGuiDockNodeFlags_PassthruCentralNode
		m_dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(m_dockspace_id, ImVec2(0, 0), dock_flags);

		
		drawMainMenu();
		if (guiData.showHeirarchy)	drawHeirarchyWindow();
		if (guiData.showViewport)	drawViewportWindow();
		if (guiData.showDemo)		ImGui::ShowDemoWindow(&guiData.showDemo);

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
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) { /* TODO: */ }
				if (ImGui::MenuItem("Open...", "Ctrl+O")) { /* TODO: */ }
				if (ImGui::MenuItem("Save", "Ctrl+S")) { /* TODO: */ }
				ImGui::Separator();
				if (ImGui::MenuItem("Exit")) { /* TODO: Add exit implementation */ }
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("View")) {
				//ImGui::MenuItem("ImGui Demo", nullptr, &guiData.showDemo);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Settings")) {
				ImGui::SliderFloat("Rotation speed (deg/s)", &rotation_speed_deg_per_s, 0.0f, 720.0f);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Windows"))
			{
				ImGui::Checkbox("Heirarchy", &guiData.showHeirarchy);
				ImGui::Checkbox("Viewport", &guiData.showViewport);
				ImGui::Checkbox("ImGui Demo", &guiData.showDemo);
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void Gui::drawHeirarchyWindow()
	{
		// Placeholder hierarchy until your GameObject graph is ready
		ImGui::Begin("Hierarchy", &guiData.showHeirarchy);
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

	void Gui::drawViewportWindow()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Viewport", &guiData.showViewport);
		ImGui::PopStyleVar();

		// How big is the render target?
		ImVec2 availableSize = ImGui::GetContentRegionAvail();
		int w = static_cast<int>(availableSize.x);
		int h = static_cast<int>(availableSize.y);
		createOrResizeFbo(w, h);

		// Mark hovered state so input can be routed properly
		m_viewportFocused = ImGui::IsWindowFocused();

		// Draw the texture. Flipped UV's if origin is bottom-left.
		if (m_colorTex)
		{
			ImGui::Image((ImTextureID)(intptr_t)m_colorTex, availableSize, ImVec2(0, 1), ImVec2(1, 0));
		}

		ImGui::End();
	}

	void Gui::createOrResizeFbo(int w, int h)
	{
		// Validate size
		if (w <= 0 || h <= 0) return;
		if (m_vpWidth == w && m_vpHeight == h && m_fbo) return;

		// Destroy existing FBO
		destroyFbo();

		glGenFramebuffers(1, &m_fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

		glGenTextures(1, &m_colorTex);
		glBindTexture(GL_TEXTURE_2D, m_colorTex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

		glGenRenderbuffers(1, &m_depthRb);
		glBindRenderbuffer(GL_RENDERBUFFER, m_depthRb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRb);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cerr << "[GUI] Framebuffer incomplete! Status = 0x" << std::hex << status << std::dec << std::endl;
		}
		else
		{
			std::cout << "[GUI] Created/Resized FBO (" << w << "x" << h << ")" << std::endl;
		}

		// Store new size
		m_vpWidth = w;
		m_vpHeight = h;
	}

	void Gui::destroyFbo()
	{
		if (m_depthRb)	{ glDeleteRenderbuffers	(1, &m_depthRb);	m_depthRb	= 0; }
		if (m_colorTex)	{ glDeleteTextures		(1, &m_colorTex);	m_colorTex	= 0; }
		if (m_fbo)		{ glDeleteFramebuffers	(1, &m_fbo);		m_fbo		= 0; }
		m_vpWidth = m_vpHeight = 0;
	}
}