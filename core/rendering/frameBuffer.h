#pragma once

#include <glad/glad.h>
#include <string>

namespace core
{
    /// <summary>
    /// Defines the types of attachments that can be associated with a framebuffer.
    /// Determines which rendering outputs (color, depth, stencil) the framebuffer will support.
    /// </summary>
    enum class AttachmentType
    {
        COLOR_ONLY,         // Framebuffer with only color attachment
        COLOR_DEPTH,        // Framebuffer with color and depth attachments
        COLOR_DEPTH_STENCIL,// Framebuffer with color, depth, and stencil attachments
        DEPTH_STENCIL       // Framebuffer with only depth and stencil attachments
    };

    /// <summary>
    /// Specifications for creating a framebuffer object.
    /// Contains all necessary parameters to configure the framebuffer's dimensions,
    /// attachment types, and texture formats.
    /// </summary>
    struct FrameBufferSpecifications
    {
        unsigned int width = 0;     /// Width of the framebuffer in pixels
        unsigned int height = 0;    /// Height of the framebuffer in pixels
        AttachmentType attachmentType = AttachmentType::COLOR_DEPTH;   /// Type of attachments to create
        GLenum colorFormat = GL_RGBA;               /// Internal format of color attachment (e.g., GL_RGBA, GL_RGB)
        GLenum depthFormat = GL_DEPTH_COMPONENT;    /// Internal format of depth attachment (e.g., GL_DEPTH_COMPONENT, GL_DEPTH24_STENCIL8)
    };

    /// <summary>
    /// Manages an OpenGL framebuffer object (FBO) with configurable attachments.
    /// Provides functionality for off-screen rendering by creating render targets
    /// with color, depth, and stencil attachments according to the specifications.
    /// </summary>
    class FrameBuffer
    {
    public:
        /// <summary>
        /// Constructs a framebuffer with the specified configuration.
        /// Creates the FBO and all required attachments based on the specifications.
        /// </summary>
        /// <param name="specs">Configuration parameters for the framebuffer</param>
        explicit FrameBuffer(const std::string& name, const FrameBufferSpecifications& specs);

        /// <summary>
        /// Constructs a default FrameBuffer object.
        /// <remark>
        /// WARNING: is empty/uninitialized.
        /// </remark>
        /// </summary>
        explicit FrameBuffer() = default;

        /// <summary>
        /// Destroys the framebuffer and releases all associated OpenGL resources.
        /// </summary>
        ~FrameBuffer();

        /// <summary>
        /// Binds this framebuffer as the current render target.
        /// All subsequent rendering operations will be directed to this framebuffer's attachments.
        /// </summary>
        void Bind() const
        {
            if (!m_isValid || m_fboID == 0)
            {
                printf("[FRAMEBUFFER] ERROR: Attempting to bind invalid framebuffer '%s' (ID: %u, Valid: %d)\n",
                       m_name.c_str(), m_fboID, m_isValid);
                return;
            }

            m_currentBoundFBOName = m_name;

            glBindFramebuffer(GL_FRAMEBUFFER, m_fboID);
        }

        /// <summary>
        /// Binds this framebuffer and clears its color and depth attachments.
        /// Sets the viewport to the framebuffer's dimensions.
        /// </summary>
        /// <param name="width">The width of the framebuffer.</param>
        /// <param name="height">The height of the framebuffer.</param>
        void BindAndClear(int width, int height) const
        {
            Bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, width, height);
        }

        /// <summary>
        /// Binds this framebuffer object for read operations.
        /// </summary>
        void BindRead() const 
        { 
            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboID); 
        }

        /// <summary>
        /// Binds this framebuffer object for draw operations.
        /// </summary>
        void BindDraw() const { glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboID); }

        /// <summary>
        /// Unbinds this framebuffer, restoring the default framebuffer (typically the screen) as the render target.
        /// </summary>
        void Unbind() const { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

        static void ClearBound(int width, int height, const char* file, int line)
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, width, height);

            printf("%s (%d)\n\t[FRAMEBUFFER] Cleared currently bound framebuffer (name: %s) to w: %4i, h: %4i.\n", file, line, m_currentBoundFBOName.c_str(), width, height);
        }

#define CLEAR_BOUND(width, height) core::FrameBuffer::ClearBound(width, height, __FILE__, __LINE__)

        /// <summary>
        /// Resizes the framebuffer and recreates all attachments with the new dimensions.
        /// This will invalidate existing texture attachments and create new ones.
        /// </summary>
        /// <param name="width">The new width of the framebuffer in pixels.</param>
        /// <param name="height">The new height of the framebuffer in pixels.</param>
        void Resize(const int width, const int height);

#pragma region GetterMethods
        /// <summary>
        /// Gets the OpenGL texture ID for the color attachment.
        /// </summary>
        /// <returns>The color texture ID, or 0 if no color attachment exists.</returns>
        GLuint GetColorAttachment() const { return m_colorTexture; }

        /// <summary>
        /// Gets the OpenGL texture ID for the depth attachment.
        /// </summary>
        /// <returns>The depth texture ID, or 0 if depth is stored in a renderbuffer.</returns>
        GLuint GetDepthAttachment() const { return m_depthTexture; }

        /// <summary>
        /// Gets the OpenGL renderbuffer ID for the depth attachment.
        /// </summary>
        /// <returns>The depth renderbuffer ID, or 0 if depth is stored in a texture.</returns>
        GLuint GetDepthRenderbuffer() const { return m_depthRenderbuffer; }

        /// <summary>
        /// Gets the OpenGL framebuffer object ID.
        /// </summary>
        /// <returns>The framebuffer object ID.</returns>
        GLuint GetFBO() const { return m_fboID; }

        /// <summary>
        /// Gets the current width of the framebuffer.
        /// </summary>
        /// <returns>The width in pixels.</returns>
        unsigned int GetWidth() const { return m_specs.width; }

        /// <summary>
        /// Gets the current height of the framebuffer.
        /// </summary>
        /// <returns>The height in pixels.</returns>
        unsigned int GetHeight() const { return m_specs.height; }

        std::string GetName() const { return m_name; }

        /// <summary>
        /// Gets the complete specification structure of this framebuffer.
        /// </summary>
        /// <returns>A constant reference to the framebuffer specifications.</returns>
        const FrameBufferSpecifications& GetSpecifications() const { return m_specs; }

        /// <summary>
        /// Checks if the framebuffer is valid and complete.
        /// A framebuffer must be complete before it can be used for rendering.
        /// </summary>
        /// <returns>True if the framebuffer is complete and ready for use, false otherwise.</returns>
        bool IsValid() const { return m_isValid; }
#pragma endregion GetterMethods

    private:
        /// <summary>
        /// Creates the framebuffer object and its attachments based on specifications.
        /// </summary>
        /// <param name="w">Width of the framebuffer in pixels.</param>
        /// <param name="h">Height of the framebuffer in pixels.</param>
        void Create(const int w, const int h);

        /// <summary>
        /// Destroys all OpenGL resources associated with this framebuffer.
        /// Deletes the FBO and all texture/renderbuffer attachments.
        /// </summary>
        void Destroy();

        /// <summary>
        /// Creates and attaches a color texture to the framebuffer.
        /// </summary>
        /// <param name="w">Width of the color texture in pixels.</param>
        /// <param name="h">Height of the color texture in pixels.</param>
        void AttachColor(const int w, const int h);

        /// <summary>
        /// Creates and attaches a depth renderbuffer to the framebuffer.
        /// </summary>
        /// <param name="w">Width of the depth renderbuffer in pixels.</param>
        /// <param name="h">Height of the depth renderbuffer in pixels.</param>
        void AttachDepth(const int w, const int h);

        /// <summary>
        /// Creates and attaches a combined depth-stencil renderbuffer to the framebuffer.
        /// </summary>
        /// <param name="w">Width of the depth-stencil renderbuffer in pixels.</param>
        /// <param name="h">Height of the depth-stencil renderbuffer in pixels.</param>
        void AttachDepthStencil(const int w, const int h);

        /// <summary>
        /// Creates and attaches a depth texture to the framebuffer.
        /// Unlike renderbuffers, depth textures can be sampled in shaders.
        /// </summary>
        /// <param name="w">Width of the depth texture in pixels.</param>
        /// <param name="h">Height of the depth texture in pixels.</param>
        void AttachDepthTexture(const int w, const int h);

        static std::string m_currentBoundFBOName;
        std::string m_name;
        FrameBufferSpecifications m_specs;  // Configuration specifications for this framebuffer
        GLuint m_fboID = 0;                 // OpenGL framebuffer object ID
        GLuint m_colorTexture = 0;          // OpenGL texture ID for color attachment
        GLuint m_depthTexture = 0;          // OpenGL texture ID for depth attachment (if used)
        GLuint m_depthRenderbuffer = 0;     // OpenGL renderbuffer ID for depth attachment (if used)
        bool m_isValid = false;             // Indicates whether the framebuffer is complete and valid
    };
} // namespace core