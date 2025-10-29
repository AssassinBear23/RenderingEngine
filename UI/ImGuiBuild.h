#pragma once

#include <imgui.h>
#include <glad/glad.h>
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
		//GuiData guiData;
		GLFWwindow* m_window = nullptr;
		bool m_initialized = false;
		unsigned int m_dockspace_id = 0;

		// FBO
		unsigned int m_fbo = 0;				// framebuffer object
		unsigned int m_colorTex = 0;		// color texture
		unsigned int m_depthRb = 0;			// depth renderbuffer
		int m_vpWidth = 0, m_vpHeight = 0;	// Viewport size
		bool m_viewportFocused = false;

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
		void drawHeirarchyWindow();
		void drawViewportWindow();
		
		// Getters & Setters
		bool viewportFocused() const { return m_viewportFocused; }
		GLuint getFramebufferId() const { return m_fbo; }
		int getViewportWidth() const { return m_vpWidth; }
		int getViewportHeight() const { return m_vpHeight; }
		ImVec2 getViewportSize() const { return ImVec2(static_cast<float>(m_vpWidth), static_cast<float>(m_vpHeight)); }

	private:
		void createOrResizeFbo(int w, int h);
		void destroyFbo();
	};

	/*struct GuiData
	{

	};*/
}