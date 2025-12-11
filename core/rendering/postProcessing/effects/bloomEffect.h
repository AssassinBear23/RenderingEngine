#pragma once

#include "../postProcessingEffectBase.h"

namespace core
{
    namespace postProcessing
    {
        class BloomEffect : public PostProcessingEffectBase
        {
        public:
            BloomEffect();
            int GetPassCount() const { return 1 + (m_blurAmount * 2); }
            void Apply(GLuint inputTexture, core::FrameBuffer& outputFBO, const int width, const int height, unsigned int passIndex = 0) override;
            void DrawGui() override;
        private:
            std::shared_ptr<core::Material> m_blurMaterial;
            std::shared_ptr<core::Material> m_brightPassMaterial;
            int m_blurAmount = 5;
            float m_blurThreshold = 1.0f;
            float m_intensity = 1.0f;
        };
    } // namespace postProcessing
} // namespace core