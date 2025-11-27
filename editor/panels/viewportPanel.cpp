#include "ViewportPanel.h"
#include <iostream>

namespace editor
{
    ViewportPanel::~ViewportPanel() { destroyFbo(); }

    ViewportPanel::ViewportPanel(Editor& editor)
        : Panel("Viewport", true)
    {
        // let the editor know “I’m the viewport”
        editor.m_viewport = this;
    }

    /// <summary>
    /// Ensures the framebuffer object (FBO) is properly sized for the viewport.
    /// Creates or recreates the FBO with color and depth attachments if dimensions change.
    /// </summary>
    /// <param name="w">The desired width of the framebuffer in pixels</param>
    /// <param name="h">The desired height of the framebuffer in pixels</param>
    void ViewportPanel::ensureFboSized(int w, int h) {
        // Validate size - reject invalid dimensions
        if (w <= 0 || h <= 0) return;
        
        // Early exit if FBO already exists with correct dimensions
        if (m_vpWidth == w && m_vpHeight == h && m_fbo) return;

        // Destroy existing FBO to prevent resource leaks
        destroyFbo();

        // Create the framebuffer object - this is the container for our render targets
        glGenFramebuffers(1, &m_fbo);
        // Bind as the current framebuffer for subsequent operations
        glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

        // === COLOR ATTACHMENT SETUP ===
        // Generate the color texture that will store the rendered image
        glGenTextures(1, &m_colorTex);
        // Bind as the active 2D texture for configuration
        glBindTexture(GL_TEXTURE_2D, m_colorTex);
        // Allocate storage: RGBA8 format (8 bits per channel), no initial data
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        // Set filtering mode for minification (when texture is smaller than screen space)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        // Set filtering mode for magnification (when texture is larger than screen space)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Attach the texture to the FBO's first color attachment point
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

        // === DEPTH/STENCIL ATTACHMENT SETUP ===
        // Generate a renderbuffer for depth and stencil data (not sampled as texture)
        glGenRenderbuffers(1, &m_depthRb);
        // Bind as the active renderbuffer for configuration
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthRb);
        // Allocate storage: combined 24-bit depth + 8-bit stencil format
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        // Attach the renderbuffer to the FBO's depth-stencil attachment point
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRb);

        // === VALIDATION ===
        // Check if framebuffer configuration is valid and complete
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            // Log error with framebuffer status code for debugging
            std::cerr << "[GUI] Framebuffer incomplete! Status = 0x" << std::hex << status << std::dec << std::endl;
            // Clean up the incomplete FBO
            destroyFbo();

        }
        else
            // Log successful creation/resize
            std::cout << "[GUI] Created/Resized FBO (" << w << "x" << h << ")" << std::endl;


        // Unbind framebuffer - return to default framebuffer (window)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Store new dimensions for future comparisons
        m_vpWidth = w;
        m_vpHeight = h;
    }

    void ViewportPanel::destroyFbo() {
        if (m_colorTex) { glDeleteTextures(1, &m_colorTex);		m_colorTex = 0; }
        if (m_depthRb) { glDeleteRenderbuffers(1, &m_depthRb);	m_depthRb = 0; }
        if (m_fbo) { glDeleteFramebuffers(1, &m_fbo);		m_fbo = 0; }
        m_vpWidth = m_vpHeight = 0;
    }

    void ViewportPanel::draw(EditorContext& /*ctx*/) {
        if (!isVisible) return;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport", &isVisible);
        ImGui::PopStyleVar();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        int w = (int)avail.x;
        int h = (int)avail.y;
        ensureFboSized(w, h);

        m_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        // Draw the color attachment (flip v)
        if (m_colorTex) {
            ImGui::Image((ImTextureID)(intptr_t)m_colorTex, avail, ImVec2(0, 1), ImVec2(1, 0));
        }

        ImGui::End();
    }
}