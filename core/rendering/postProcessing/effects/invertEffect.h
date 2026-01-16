#pragma once

#include "../postProcessingEffectBase.h"
#include <memory>

namespace core
{
    class FrameBuffer;

    namespace postProcessing
    {
        class PostProcessingManager;
        
        /// <summary>
        /// Post-processing effect that inverts the colors of the rendered scene.
        /// This effect converts each RGB color to its complement by subtracting from white (1.0, 1.0, 1.0).
        /// </summary>
        class InvertEffect : public PostProcessingEffectBase
        {
        public:
            /// <summary>
            /// Constructs an InvertEffect instance.
            /// </summary>
            /// <param name="manager">Weak pointer to the PostProcessingManager that owns this effect.</param>
            InvertEffect(std::weak_ptr<PostProcessingManager> manager);
            
            /// <summary>
            /// Applies the color inversion effect to the input framebuffer and writes the result to the output framebuffer.
            /// Each pixel's RGB values are inverted using the formula: output = 1.0 - input.
            /// </summary>
            /// <param name="inputFBO">The framebuffer containing the source image to be processed.</param>
            /// <param name="outputFBO">The framebuffer where the inverted image will be written.</param>
            /// <param name="width">The width of the viewport in pixels.</param>
            /// <param name="height">The height of the viewport in pixels.</param>
            void Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height) override;
            
            /// <summary>
            /// Renders the ImGui interface for this effect, allowing runtime configuration and toggling.
            /// Provides controls for enabling/disabling the invert effect.
            /// </summary>
            void DrawGui() override;
        };
    } // namespace postProcessing
} // namespace core