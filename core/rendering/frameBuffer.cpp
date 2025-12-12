#include "frameBuffer.h"
#include <cstdio>

namespace core
{
    FrameBuffer::FrameBuffer(const FrameBufferSpecifications& specs)
        : m_specs(specs)
    {
        Create(m_specs.width, m_specs.height);
    }

    FrameBuffer::~FrameBuffer() { Destroy(); m_specs.width = m_specs.height = 0; }

    void FrameBuffer::Resize(const int width, const int height)
    {
        // Avoid unnecessary resize operations
        if ((width <= 0 || height <= 0) || (m_specs.width == width && m_specs.height == height))
            return;

        m_specs.width = width;
        m_specs.height = height;
        Destroy();
        Create(width, height);
    }

    void FrameBuffer::Create(const int w, const int h)
    {
        glGenFramebuffers(1, &m_fboID);
        glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);

        switch (m_specs.attachmentType)
        {
        case AttachmentType::COLOR_ONLY:
            AttachColor(w, h);
            break;
        case AttachmentType::COLOR_DEPTH:
            AttachColor(w, h);
            AttachDepth(w, h);
            break;
        case AttachmentType::COLOR_DEPTH_STENCIL:
            AttachColor(w, h);
            AttachDepthStencil(w, h);
            break;
        case AttachmentType::DEPTH_STENCIL:
            AttachDepthTexture(w, h);
            break;
        }

        // Check framebuffer completeness
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        m_isValid = (status == GL_FRAMEBUFFER_COMPLETE);

        if (!m_isValid)
        {
            // Log error if framebuffer is incomplete
            printf("[FRAMEBUFFER] Framebuffer incomplete! Status: 0x%X\n", status);
            Destroy();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::Destroy()
    {
        if (m_colorTexture)
        {
            glDeleteTextures(1, &m_colorTexture);
            m_colorTexture = 0;
        }
        if (m_depthRenderbuffer)
        {
            glDeleteRenderbuffers(1, &m_depthRenderbuffer);
            m_depthRenderbuffer = 0;
        }
        if (m_fboID)
        {
            glDeleteFramebuffers(1, &m_fboID);
            m_fboID = 0;
        }
        m_isValid = false;
    }

    void FrameBuffer::AttachColor(const int w, const int h)
    {
        glGenTextures(1, &m_colorTexture);
        glBindTexture(GL_TEXTURE_2D, m_colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, m_specs.colorFormat, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
    }

    void FrameBuffer::AttachDepth(const int w, const int h)
    {
        glGenRenderbuffers(1, &m_depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    }

    void FrameBuffer::AttachDepthStencil(const int w, const int h)
    {
        glGenRenderbuffers(1, &m_depthRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRenderbuffer);
    }

    void FrameBuffer::AttachDepthTexture(const int w, const int h)
    {
        glGenTextures(1, &m_depthTexture);
        glBindTexture(GL_TEXTURE_2D, m_depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, m_specs.depthFormat, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    }

} // namespace core