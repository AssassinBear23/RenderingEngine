#pragma once
#include "../frameBuffer.h"
#include <memory>
#include <vector>

namespace core
{
    namespace postProcessing
    {
        class PostProcessingEffectBase;

        /// <summary>
        /// Manages the post-processing effect stack for rendering.
        /// Handles the execution order and application of multiple post-processing effects.
        /// Maintains both a complete list of all effects and a sorted list of currently enabled effects.
        /// </summary>
        class PostProcessingManager : public std::enable_shared_from_this<PostProcessingManager>
        {
        public:
            /// <summary>
            /// Constructs a new PostProcessingManager instance.
            /// </summary>
            PostProcessingManager();
            
            /// <summary>
            /// Default destructor.
            /// </summary>
            ~PostProcessingManager() = default;

            /// <summary>
            /// Processes the entire effect stack, applying all enabled effects sequentially.
            /// </summary>
            /// <param name="inputBuffer">The source framebuffer containing the initial render output.</param>
            /// <param name="outputBuffer">The destination framebuffer where the final processed output is written.</param>
            /// <param name="width">The width of the framebuffers in pixels.</param>
            /// <param name="height">The height of the framebuffers in pixels.</param>
            void ProcessStack(FrameBuffer& inputBuffer, core::FrameBuffer& outputBuffer, const unsigned int width, const unsigned int height);

            /// <summary>
            /// Adds a new post-processing effect to the manager.
            /// </summary>
            /// <param name="effect">Shared pointer to the effect to add.</param>
            /// <returns>True if the effect was successfully added, false otherwise.</returns>
            bool AddEffect(const std::shared_ptr<PostProcessingEffectBase> effect);

            /// <summary>
            /// Enables a previously added effect and updates the enabled effects list.
            /// </summary>
            /// <param name="effect">Shared pointer to the effect to enable.</param>
            void EnableEffect(const std::shared_ptr<PostProcessingEffectBase> effect);
            
            /// <summary>
            /// Disables an effect and updates the enabled effects list.
            /// </summary>
            /// <param name="effect">Shared pointer to the effect to disable.</param>
            void DisableEffect(const std::shared_ptr<PostProcessingEffectBase> effect);

            /// <summary>
            /// Initializes the post-processing manager and its resources.
            /// Should be called before using the manager to process effects.
            /// </summary>
            void Initialize();

            /// <summary>
            /// Retrieves all registered effects (both enabled and disabled).
            /// </summary>
            /// <returns>A vector containing shared pointers to all registered effects.</returns>
            std::vector<std::shared_ptr<PostProcessingEffectBase>> GetEffects() const { return m_effects; }

        private:
            /// <summary>
            /// Sorts the enabled effects list based on their execution priority.
            /// Ensures effects are applied in the correct order during processing.
            /// </summary>
            void SortEnabledEffects();

            /// <summary>
            /// Temporary framebuffer used for ping-pong rendering between effects.
            /// </summary>
            core::FrameBuffer tempFBO;

            /// <summary>
            /// Complete list of all registered post-processing effects.
            /// </summary>
            std::vector<std::shared_ptr<PostProcessingEffectBase>> m_effects;
            
            /// <summary>
            /// Sorted list of currently enabled effects, ready for processing.
            /// </summary>
            std::vector<std::shared_ptr<PostProcessingEffectBase>> m_enabledEffects;
        };
    } // namespace postProcessing
} // namespace core