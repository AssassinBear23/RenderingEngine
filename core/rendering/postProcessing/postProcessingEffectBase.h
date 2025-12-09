#pragma once

#include <glad/glad.h>
#include <memory>
#include <string>

namespace core // Forward declaration
{
    class Material;
    class FrameBuffer;
}

namespace core
{
    namespace postProcessing
    {
        /// <summary>
        /// Base class for all post-processing effects in the rendering pipeline.
        /// Provides common functionality for managing materials, enable states, and applying effects.
        /// </summary>
        class PostProcessingEffectBase
        {
        public:
            /// <summary>
            /// Constructs a new post-processing effect with the specified name and material.
            /// </summary>
            /// <param name="name">The unique name identifier for this post-processing effect.</param>
            /// <param name="material">The material containing the shader and properties used for this effect.</param>
            PostProcessingEffectBase(const std::string& name, std::shared_ptr<core::Material> material);
            
            /// <summary>
            /// Gets whether this post-processing effect is currently enabled.
            /// </summary>
            /// <returns>True if the effect is enabled, false otherwise.</returns>
            bool IsEnabled() const { return m_enabled; }
            
            /// <summary>
            /// Sets the enabled state of this post-processing effect.
            /// </summary>
            /// <param name="enabled">True to enable the effect, false to disable it.</param>
            void SetEnabled(const bool enabled) { m_enabled = enabled; }

#pragma region GetterMethods
            /// <summary>
            /// Gets the material used by this post-processing effect.
            /// </summary>
            /// <returns>A shared pointer to the material containing the effect's shader and properties.</returns>
            std::shared_ptr<core::Material> GetMaterial() const { return m_material; }
            
            /// <summary>
            /// Gets the name identifier of this post-processing effect.
            /// </summary>
            /// <returns>A constant reference to the effect's name string.</returns>
            const std::string& GetName() const { return m_name; }
#pragma endregion GetterMethods
        protected:
            /// <summary>
            /// Applies the post-processing effect to the input texture and renders the result to the output framebuffer.
            /// Override this method in derived classes to implement custom effect behavior.
            /// </summary>
            /// <param name="inputTexture">The OpenGL texture ID of the input texture to process.</param>
            /// <param name="outputFBO">The framebuffer object to render the processed result into.</param>
            /// <param name="width">The width of the rendering viewport in pixels.</param>
            /// <param name="height">The height of the rendering viewport in pixels.</param>
            virtual void Apply(GLuint inputTexture, core::FrameBuffer& outputFBO, const int width, const int height);
            
            /// <summary>
            /// Draws the GUI elements for this post-processing effect.
            /// Override this method in derived classes to provide custom ImGui controls for effect parameters.
            /// </summary>
            virtual void DrawGui();
        private:
            /// <summary>
            /// Flag indicating whether this effect is currently enabled.
            /// </summary>
            bool m_enabled = false;
            
            /// <summary>
            /// The material containing the shader and properties used for this effect.
            /// </summary>
            std::shared_ptr<core::Material> m_material;
            
            /// <summary>
            /// The unique name identifier for this post-processing effect.
            /// </summary>
            std::string m_name;
        };
    } // namespace postProcessing
} // namespace core