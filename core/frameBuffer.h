#pragma once

#include <glad/glad.h>

namespace core
{
    enum class AttachmentType
    {
        COLOR_ONLY,
        COLOR_DEPTH,
        COLOR_DEPTH_STENCIL,
        DEPTH_STENCIL
    };

    struct FrameBufferSpecifications
    {
        unsigned int width = 0;     // Width of the framebuffer in pixels
        unsigned int height = 0;    // Height of the framebuffer in pixels
        AttachmentType attachementType = AttachmentType::COLOR_DEPTH;   // Type of attachments
        GLenum colorFormat = GL_RGBA;               // format of color attachment
        GLenum depthFormat = GL_DEPTH_COMPONENT;    // format of depth attachment
    };

    class FrameBuffer
    {
    public:
        explicit FrameBuffer(const FrameBufferSpecifications& specs);
        ~FrameBuffer();

        /// <summary>
        /// Binds this framebuffer as the current render target.
        /// All subsequent rendering operations will be directed to this framebuffer's attachments.
        /// </summary>
        void Bind() const;
        
        /// <summary>
        /// Unbinds this framebuffer, restoring the default framebuffer (typically the screen) as the render target.
        /// </summary>
        void Unbind() const;
        
        /// <summary>
        /// Resizes the framebuffer and recreates all attachments with the new dimensions.
        /// This will invalidate existing texture attachments and create new ones.
        /// </summary>
        /// <param name="width">The new width of the framebuffer in pixels.</param>
        /// <param name="height">The new height of the framebuffer in pixels.</param>
        void Resize(const int width,const int height);

        // Getters and Setters
        GLuint GetColorAttachment() const { return m_colorTexture; }
        GLuint GetDepthAttachment() const { return m_depthTexture; }
        GLuint GetDepthRenderbuffer() const { return m_depthRenderbuffer; }
        GLuint GetFBO() const { return m_fboID; }
        unsigned int GetWidth() const { return m_specs.width; }
        unsigned int GetHeight() const { return m_specs.height; }
        const FrameBufferSpecifications& GetSpecifications() const { return m_specs; }
        
        // Check if the framebuffer is valid/complete
        bool IsValid() const { return m_isValid; }
    private:
        void Create(const int w, const int h);
        void Destroy();
        void AttachColor(const int w, const int h);
        void AttachDepth(const int w, const int h);
        void AttachDepthStencil(const int w, const int h);
        void AttachDepthTexture(const int w, const int h);

        FrameBufferSpecifications m_specs;
        GLuint m_fboID = 0;
        GLuint m_colorTexture = 0;
        GLuint m_depthTexture = 0;
        GLuint m_depthRenderbuffer = 0;
        bool m_isValid = false;
    };
} // namespace core