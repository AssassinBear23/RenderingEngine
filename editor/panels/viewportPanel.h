#pragma once
#include "../editor.h"
#include "../panel.h"
#include <core/rendering/frameBuffer.h>
#include <glad/glad.h>

namespace editor
{
    class ViewportPanel : public Panel
    {
    public:
        explicit ViewportPanel(Editor& editor);
        ~ViewportPanel();

        void draw(EditorContext& ctx) override;

        // Expose render target to app
        GLuint framebuffer() const { return m_frameBuffer.GetFBO(); }
        unsigned int GetWidth() const { return m_frameBuffer.GetWidth(); }
        unsigned int GetHeight() const { return m_frameBuffer.GetHeight(); }
        bool   isFocused() const { return m_focused; }

    private:
        core::FrameBuffer m_frameBuffer{ {800, 600, core::AttachmentType::COLOR_DEPTH}};
        bool m_focused = false;
    };
}
