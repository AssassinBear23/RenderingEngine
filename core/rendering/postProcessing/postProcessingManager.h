#pragma once

#include <glad/glad.h>
#include <memory>
#include <vector>

namespace core
{
    class FrameBuffer;

    namespace postProcessing
    {
        class PostProcessingEffectBase;

        class PostProcessingManager
        {
        public:
            PostProcessingManager() = default;
            ~PostProcessingManager() = default;

            void ProcessStack(FrameBuffer& inputBuffer, core::FrameBuffer& outputBuffer, const unsigned int width, const unsigned int height);

            bool AddEffect(const std::shared_ptr<PostProcessingEffectBase> effect);
            bool RemoveEffect(const std::shared_ptr<PostProcessingEffectBase> effect);

            std::vector<std::shared_ptr<PostProcessingEffectBase>> GetEffects() const { return m_effects; }

        private:
            void RenderQuad(const unsigned int width, const unsigned int height);

            std::vector<std::shared_ptr<PostProcessingEffectBase>> m_effects;
        };
    } // namespace postProcessing
} // namespace core