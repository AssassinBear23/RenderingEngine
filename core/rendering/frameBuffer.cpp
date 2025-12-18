#include "frameBuffer.h"
#include <cstdio>
#include <string>

namespace core
{
    std::string FrameBuffer::m_currentBoundFBOName = "Not Set";

    FrameBuffer::FrameBuffer(const std::string& name, const FrameBufferSpecifications& specs)
        : m_specs(specs), m_name(name)
    {
        Create(m_specs.width, m_specs.height);
    }

    FrameBuffer::~FrameBuffer() { Destroy(); m_specs.width = m_specs.height = 0; }

    void FrameBuffer::Resize(const int width, const int height)
    {
        // Avoid unnecessary resize operations
        if ((width <= 0 || height <= 0) || (m_specs.width == width && m_specs.height == height))
            return;

        printf("[FRAMEBUFFER] Resizing %-20s to w: %4i, h: %4i.\n", m_name.c_str(), width, height);

        m_specs.width = width;
        m_specs.height = height;
        Destroy();
        Create(width, height);
    }

    void FrameBuffer::Create(const int w, const int h)
    {
        // Ensure we start clean
        if (m_fboID != 0)
        {
            printf("[FRAMEBUFFER] WARNING: Create() called with existing FBO ID %u for '%s'. Destroying first.\n", 
                   m_fboID, m_name.c_str());
            Destroy();
        }

        glGenFramebuffers(1, &m_fboID);
        
        // Verify the FBO was generated
        if (m_fboID == 0)
        {
            printf("[FRAMEBUFFER] ERROR: glGenFramebuffers failed for '%s'\n", m_name.c_str());
            m_isValid = false;
            return;
        }
        
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
            printf("[FRAMEBUFFER] Framebuffer '%s' incomplete! Status: 0x%X\n", m_name.c_str(), status);
            Destroy();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::Destroy()
    {
        // Delete all color textures
        if (!m_colorTextures.empty())
        {
            glDeleteTextures(static_cast<GLsizei>(m_colorTextures.size()), m_colorTextures.data());
            m_colorTextures.clear();
        }
        if (m_depthTexture)
        {
            glDeleteTextures(1, &m_depthTexture);
            m_depthTexture = 0;
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
        unsigned int numAttachments = m_specs.numColorAttachments;
        if (numAttachments == 0) numAttachments = 1;
        
        m_colorTextures.resize(numAttachments);
        std::vector<GLenum> drawBuffers(numAttachments);
        
        for (unsigned int i = 0; i < numAttachments; ++i)
        {
            // Generate and configure texture
            glGenTextures(1, &m_colorTextures[i]);
            glBindTexture(GL_TEXTURE_2D, m_colorTextures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            
            // Attach to framebuffer
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, 
                                   GL_TEXTURE_2D, m_colorTextures[i], 0);
            
            drawBuffers[i] = GL_COLOR_ATTACHMENT0 + i;
            
            printf("[FRAMEBUFFER] Attached color texture %u to '%s' at GL_COLOR_ATTACHMENT%u\n", 
                   m_colorTextures[i], m_name.c_str(), i);
        }
        
        // CRITICAL: Tell OpenGL which color attachments to use
        glDrawBuffers(static_cast<GLsizei>(numAttachments), drawBuffers.data());
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);
    }

    FrameBuffer::FrameBuffer(FrameBuffer&& other) noexcept
        : m_name(std::move(other.m_name))
        , m_specs(std::move(other.m_specs))
        , m_fboID(other.m_fboID)
        , m_colorTextures(std::move(other.m_colorTextures))
        , m_depthTexture(other.m_depthTexture)
        , m_depthRenderbuffer(other.m_depthRenderbuffer)
        , m_isValid(other.m_isValid)
    {
        // Reset the moved-from object
        other.m_fboID = 0;
        other.m_colorTextures.clear();
        other.m_depthTexture = 0;
        other.m_depthRenderbuffer = 0;
        other.m_isValid = false;
    }

    FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) noexcept
    {
        if (this != &other)
        {
            Destroy(); // Clean up current resources

            m_name = std::move(other.m_name);
            m_specs = std::move(other.m_specs);
            m_fboID = other.m_fboID;
            m_colorTextures = std::move(other.m_colorTextures);
            m_depthTexture = other.m_depthTexture;
            m_depthRenderbuffer = other.m_depthRenderbuffer;
            m_isValid = other.m_isValid;

            // Reset the moved-from object
            other.m_fboID = 0;
            other.m_colorTextures.clear();
            other.m_depthTexture = 0;
            other.m_depthRenderbuffer = 0;
            other.m_isValid = false;
        }
        return *this;
    }

} // namespace core