#pragma once

#include "../../../property.h"
#include "../postProcessingEffectBase.h"
#include <memory>

namespace core
{
    class FrameBuffer;
    class Material;
    class Shader;

    namespace postProcessing
    {
        class PostProcessingManager;

        enum class BloomDebugMode
        {
            None,           // Normal bloom rendering
            ThresholdOnly,  // Show only the bright pass (threshold) result
            BlurOnly        // Show only the blur result without combining
        };

        class BloomEffect : public PostProcessingEffectBase
        {
        public:
            BloomEffect(std::weak_ptr<PostProcessingManager> manager);

            int GetPassCount() const override;
            void Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height) override;
            void DrawGui() override;

        private:
            std::shared_ptr<Material> m_blurMaterial;
            std::shared_ptr<Material> m_brightPassMaterial;
            std::shared_ptr<Material> m_compositeMaterial;
            std::shared_ptr<Shader> m_brightPassShader;
            std::shared_ptr<Shader> m_blurShader;
            std::shared_ptr<Shader> m_compositeShader;

            
            int m_blurAmount = 5;
            Property<float> m_bloomThreshold = 0.2f;
            float m_intensity = 1.0f;
            BloomDebugMode m_debugMode = BloomDebugMode::None;

            FrameBuffer tempFBO_1;
            FrameBuffer tempFBO_2;
        };
    } // namespace postProcessing
} // namespace core