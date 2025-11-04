#include "ViewportPanel.h"

ViewportPanel::~ViewportPanel() { destroyFbo(); }

void ViewportPanel::ensureFboSized(int w, int h) {
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

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cerr << "[GUI] Framebuffer incomplete! Status = 0x" << std::hex << status << std::dec << std::endl;
		destroyFbo();
	
	}
	else
		std::cout << "[GUI] Created/Resized FBO (" << w << "x" << h << ")" << std::endl;
	

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Store new size
	m_vpWidth = w;
	m_vpHeight = h;
}

void ViewportPanel::destroyFbo() {
	if (m_depthRb) { glDeleteRenderbuffers(1, &m_depthRb); m_depthRb = 0; }
	if (m_colorTex) { glDeleteTextures(1, &m_colorTex);     m_colorTex = 0; }
	if (m_fbo) { glDeleteFramebuffers(1, &m_fbo);      m_fbo = 0; }
	m_vpWidth = m_vpHeight = 0;
}

void ViewportPanel::draw(EditorContext& /*ctx*/) {
	if (!*visiblePtr()) return;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::Begin("Viewport", visiblePtr());
	ImGui::PopStyleVar();

	// Resize FBO to available content size
	ImVec2 avail = ImGui::GetContentRegionAvail();


	ImGui::Text("w=%d h=%d fbo=%u tex=%u", (int)avail.x, (int)avail.y, m_fbo, m_colorTex);

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
