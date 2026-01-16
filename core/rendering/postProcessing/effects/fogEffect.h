#pragma once

#include "../postProcessingEffectBase.h"
#include "../../../property.h"
#include <memory>
#include <glm/glm.hpp>

namespace core
{
    namespace postProcessing
    {
        /// <summary>
        /// Defines the fog calculation mode used for depth-based fog effects.
        /// </summary>
        enum class FogMode
        {
            /// <summary>Linear fog interpolation between start and end distances.</summary>
            Linear = 0,
            /// <summary>Exponential fog falloff based on density.</summary>
            Exponential = 1,
            /// <summary>Exponential squared fog falloff for denser fog.</summary>
            ExponentialSquared = 2
        };

        /// <summary>
        /// Debug visualization modes for fog effect development and tuning.
        /// </summary>
        enum class FogDebugMode
        {
            /// <summary>Normal rendering with fog applied.</summary>
            Normal = 0,
            /// <summary>Visualize raw depth buffer values.</summary>
            RawDepth = 1,
            /// <summary>Visualize linearized depth values.</summary>
            LinearDepth = 2,
            /// <summary>Visualize computed fog factor.</summary>
            FogFactor = 3
        };

        /// <summary>
        /// Fog post-processing effect that applies depth-based fog to the scene.
        /// Supports multiple fog modes (linear, exponential, exponential squared) and debug visualizations.
        /// </summary>
        class FogEffect : public PostProcessingEffectBase
        {
        public:
            /// <summary>
            /// Constructs a new fog effect with the specified post-processing manager.
            /// </summary>
            /// <param name="manager">Weak pointer to the parent post-processing manager.</param>
            FogEffect(std::weak_ptr<PostProcessingManager> manager);

            /// <summary>
            /// Applies the fog effect using depth information from the scene.
            /// Reads depth buffer from input FBO and composites fog color based on distance.
            /// </summary>
            /// <param name="inputFBO">The previous effect's output framebuffer containing color and depth.</param>
            /// <param name="outputFBO">The output framebuffer where the fogged result will be written.</param>
            /// <param name="width">Viewport width in pixels.</param>
            /// <param name="height">Viewport height in pixels.</param>
            void Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height) override;

            /// <summary>
            /// Renders the ImGui interface for adjusting fog parameters.
            /// Provides controls for fog color, density, range, mode selection, and debug visualization.
            /// </summary>
            void DrawGui() override;

        private:
            /// <summary>RGB color of the fog. Default is light blue-grey (0.5, 0.6, 0.7).</summary>
            Property<glm::vec3> m_fogColor = glm::vec3(0.5f, 0.6f, 0.7f);
            
            /// <summary>Fog density for exponential fog modes. Higher values create denser fog. Default is 0.05.</summary>
            Property<float> m_fogDensity = 0.05f;
            
            /// <summary>Start distance for linear fog mode. Fog begins at this distance from camera. Default is 10 units.</summary>
            Property<float> m_fogStart = 10.0f;
            
            /// <summary>End distance for linear fog mode. Fog reaches maximum density at this distance. Default is 100 units.</summary>
            Property<float> m_fogEnd = 100.0f;
            
            /// <summary>Current fog calculation mode (Linear, Exponential, or ExponentialSquared).</summary>
            Property<FogMode> m_fogMode = FogMode::Linear;
            
            /// <summary>Current debug visualization mode for development and tuning.</summary>
            Property<FogDebugMode> m_debugMode = FogDebugMode::Normal;
            
            /// <summary>Near clipping plane distance used for depth linearization. Default is 0.1 units.</summary>
            float m_nearPlane = 0.1f;
            
            /// <summary>Far clipping plane distance used for depth linearization. Default is 1000 units.</summary>
            float m_farPlane = 1000.0f;
        };
    }
}