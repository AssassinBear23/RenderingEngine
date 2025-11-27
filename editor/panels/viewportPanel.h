#pragma once
#include "../editor.h"
#include "../panel.h"
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
        GLuint framebuffer() const { return m_fbo; }
        int    width() const { return m_vpWidth; }
        int    height() const { return m_vpHeight; }
        bool   isFocused() const { return m_focused; }

    private:
        void ensureFboSized(int w, int h);
        void destroyFbo();

        GLuint m_fbo = 0;
        GLuint m_colorTex = 0;
        GLuint m_depthRb = 0;
        int m_vpWidth = 0, m_vpHeight = 0;
        bool m_focused = false;
    };
}
