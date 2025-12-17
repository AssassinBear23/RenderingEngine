#pragma once

#include "../postProcessingEffectBase.h"
#include <glad/glad.h>
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

            // Debug getters
            BloomDebugMode GetDebugMode() const { return m_debugMode; }
            float GetThreshold() const { return m_bloomThreshold; }
            float GetIntensity() const { return m_intensity; }
            int GetBlurAmount() const { return m_blurAmount; }

            // Debug setters
            void SetDebugMode(BloomDebugMode mode) { m_debugMode = mode; }
            void SetThreshold(float threshold) { m_bloomThreshold = threshold; }
            void SetIntensity(float intensity) { m_intensity = intensity; }
            void SetBlurAmount(int amount) { m_blurAmount = amount; }

        private:
            std::shared_ptr<Material> m_blurMaterial;
            std::shared_ptr<Material> m_brightPassMaterial;
            std::shared_ptr<Material> m_compositeMaterial;
            std::shared_ptr<Shader> m_brightPassShader;
            std::shared_ptr<Shader> m_blurShader;
            std::shared_ptr<Shader> m_compositeShader;

            
            int m_blurAmount = 5;
            float m_bloomThreshold = 0.2f;
            float m_intensity = 1.0f;
            BloomDebugMode m_debugMode = BloomDebugMode::None;

            FrameBuffer tempFBO_1;
            FrameBuffer tempFBO_2;
        };
    } // namespace postProcessing
} // namespace core