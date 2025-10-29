#pragma once

#include <imgui.h>
#include <glad/glad.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

namespace UI
{
	/// <summary>
	/// Holds the state of the GUI, including visibility of various panels.
	/// </summary>
	struct GuiData
	{
	public:
		bool showDemo = false;
		bool showHeirarchy = true;
		bool showViewport = true;
	};

	/// <summary>
	/// Dear ImGui-based GUI wrapper that manages initialization, frame lifecycle, docking, and a render-to-texture viewport.
	/// </summary>
	class Gui
	{
		// Fields
	public:
		/// <summary>
		/// Rotation speed (degrees/second) used by sample content displayed in the viewport.
		/// </summary>
		float rotation_speed_deg_per_s = 10.0f;
		/// <summary>
		/// The GUI state data controlling visibility of panels and other options.
		/// </summary>
		GuiData guiData;
	private:
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

		/// <summary>
		/// Default constructor; the GUI remains uninitialized until Init is called.
		/// </summary>
		Gui() = default;
		/// <summary>
		/// Destructor; ensures resources are released (equivalent to calling Shutdown if still initialized).
		/// </summary>
		~Gui();

		// Non-copyable, movable if needed
		Gui(const Gui&) = delete;
		Gui& operator=(const Gui&) = delete;

		/// <summary>
		/// Initializes Dear ImGui for GLFW and OpenGL3, configures docking, and binds to the provided window.
		/// </summary>
		/// <param name="window">Valid GLFW window handle (not owned by this class).</param>
		/// <param name="glsl_version">GLSL version string used by ImGui's OpenGL3 backend (e.g., "#version 400").</param>
		void Init(GLFWwindow* window, const char* glsl_version = "#version 400");
		/// <summary>
		/// Shuts down Dear ImGui backends and releases GPU resources (FBO, textures, renderbuffers).
		/// </summary>
		void Shutdown();

		// Per-frame

		/// <summary>
		/// Begins a new ImGui frame and sets up the main dockspace.
		/// </summary>
		void BeginFrame();

		/// <summary>
		/// Builds and draws all GUI panels and widgets for the current frame.
		/// </summary>
		void Draw();

		/// <summary>
		/// Finalizes the frame and renders ImGui draw data to the current backbuffer.
		/// </summary>
		void EndFrame();

		// Panels

		/// <summary>
		/// Draws the main menu bar (File, View, Tools, etc.).
		/// </summary>
		void drawMainMenu();
		/// <summary>
		/// Draws the hierarchy panel listing scene/entities.
		/// </summary>
		void drawHeirarchyWindow();
		/// <summary>
		/// Draws the viewport panel, rendering the scene into an FBO and handling resizing/focus.
		/// </summary>
		void drawViewportWindow();

		// Getters & Setters

		/// <summary>
		/// Returns whether the viewport window is currently focused.
		/// </summary>
		/// <returns>True if the viewport window has keyboard focus; otherwise, false.</returns>
		bool viewportFocused() const { return m_viewportFocused; }
		/// <summary>
		/// Returns the OpenGL ID of the off-screen framebuffer used by the viewport.
		/// </summary>
		/// <returns>Framebuffer object ID (0 if not created).</returns>
		GLuint getFramebufferId() const { return m_fbo; }
		/// <summary>
		/// Returns the viewport width in pixels.
		/// </summary>
		/// <returns>Viewport width (pixels).</returns>
		int getViewportWidth() const { return m_vpWidth; }
		/// <summary>
		/// Returns the viewport height in pixels.
		/// </summary>
		/// <returns>Viewport height (pixels).</returns>
		int getViewportHeight() const { return m_vpHeight; }
		/// <summary>
		/// Returns the viewport size as an ImVec2 (width, height).
		/// </summary>
		/// <returns>Viewport size in pixels.</returns>
		ImVec2 getViewportSize() const { return ImVec2(static_cast<float>(m_vpWidth), static_cast<float>(m_vpHeight)); }

	private:
		/// <summary>
		/// Creates the framebuffer or resizes attachments to match the requested viewport dimensions.
		/// </summary>
		/// <param name="w">New viewport width in pixels.</param>
		/// <param name="h">New viewport height in pixels.</param>
		void createOrResizeFbo(int w, int h);
		/// <summary>
		/// Destroys the framebuffer and its attachments, freeing GPU resources.
		/// </summary>
		void destroyFbo();
	};
}