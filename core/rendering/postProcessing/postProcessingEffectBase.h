#pragma once

#include "../../property.h"
#include <memory>
#include <string>

namespace core
{
    class Material;
    class FrameBuffer;
    class Shader;

    namespace postProcessing
    {
        class PostProcessingManager;

        /// <summary>
        /// Base class for all post-processing effects in the rendering pipeline.
        /// Provides common functionality for managing materials, enable states, and applying effects.
        /// </summary>
        class PostProcessingEffectBase : public std::enable_shared_from_this<PostProcessingEffectBase>
        {
        public:
            /// <summary>
            /// Constructs a new post-processing effect with the specified name and material.
            /// </summary>
            /// <param name="name">The unique name identifier for this post-processing effect.</param>
            /// <param name="material">The material containing the shader and properties used for this effect.</param>
            PostProcessingEffectBase(const std::string& name, std::shared_ptr<Material> material, std::weak_ptr<PostProcessingManager> manager, bool requireSceneRender);

            /// <summary>
            /// Performs initialization.
            /// Called by the Manager when the effect is added to the managers list.
            /// </summary>
            void Initialize();

            virtual int GetPassCount() const { return 1; }

            /// <summary>
            /// Applies the post-processing effect to the input texture and renders the result to the output framebuffer.
            /// Override this method in derived classes to implement custom effect behavior.
            /// </summary>
            /// <param name="inputTexture">The OpenGL texture ID of the input texture to process.</param>
            /// <param name="outputFBO">The framebuffer object to render the processed result into.</param>
            /// <param name="width">The width of the rendering viewport in pixels.</param>
            /// <param name="height">The height of the rendering viewport in pixels.</param>
            virtual void Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height);

            /// <summary>
            /// Draws the GUI elements for this post-processing effect.
            /// Override this method in derived classes to provide custom ImGui controls for effect parameters.
            /// </summary>
            virtual void DrawGui() = 0;

            Property<bool> isEnabled;

#pragma region GetterMethods
            /// <summary>
            /// Gets the material used by this post-processing effect.
            /// </summary>
            /// <returns>A shared pointer to the material containing the effect's shader and properties.</returns>
            std::shared_ptr<Material> GetMaterial() const { return m_material; }

            /// <summary>
            /// Gets the name identifier of this post-processing effect.
            /// </summary>
            /// <returns>A constant reference to the effect's name string.</returns>
            const std::string& GetName() const { return m_name; }

            bool RequiresSceneRender() const { return m_requireSceneRender; }
#pragma endregion GetterMethods

        protected:
            void RenderQuad(const unsigned int width, const unsigned int height);

            /// <summary>
            /// The shader used by the material.
            /// </summary>
            std::shared_ptr<Shader> m_shader;

            /// <summary>
            /// The material containing the shader and properties used for this effect.
            /// </summary>
            std::shared_ptr<Material> m_material;

            /// <summary>
            /// The manager that owns this effect.
            /// </summary>
            std::weak_ptr<PostProcessingManager> m_manager;
        
        private:
            std::string m_name;
            bool m_requireSceneRender;
        };
    } // namespace postProcessing
} // namespace core