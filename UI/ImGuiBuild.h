#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace UI
{
	class Gui
	{
	// Fields
	public:
		bool showDemo = false;
		float rotation_speed_deg_per_s = 10.0f;
	private:
		GLFWwindow* m_window = nullptr;
		bool m_initialized = false;
		unsigned int m_dockspace_id = 0;

	// Methods
	public:
		// Constructor / Destructor
		Gui() = default;
		~Gui();

		// Non-copyable, movable if needed
		Gui(const Gui&) = delete;
		Gui& operator=(const Gui&) = delete;

		
		void Init(GLFWwindow* window, const char* glsl_version = "#version 400");
		void Shutdown();

		// Per-frame
		void BeginFrame();
		void Draw();
		void EndFrame();

		// Panels
		void drawMainMenu();
		void drawHeirarchyPanel();
	};
}