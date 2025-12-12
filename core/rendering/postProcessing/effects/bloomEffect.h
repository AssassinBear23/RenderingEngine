#pragma once

#include "../postProcessingEffectBase.h"
#include <glad/glad.h>
#include <memory>

namespace core
{
    class FrameBuffer;
    class Material;

    namespace postProcessing
    {
        enum class BloomDebugMode
        {
            None,           // Normal bloom rendering
            ThresholdOnly,  // Show only the bright pass (threshold) result
            BlurOnly        // Show only the blur result without combining
        };

        class BloomEffect : public PostProcessingEffectBase
        {
        public:
            BloomEffect();
            int GetPassCount() const override;
            void Apply(GLuint inputTexture, core::FrameBuffer& outputFBO, const int width, const int height, unsigned int passIndex = 0) override;
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
            std::shared_ptr<core::Material> m_blurMaterial;
            std::shared_ptr<core::Material> m_brightPassMaterial;
            
            int m_blurAmount = 5;
            float m_bloomThreshold = 0.2f;
            float m_intensity = 1.0f;
            BloomDebugMode m_debugMode = BloomDebugMode::None;
            
            // Store original scene texture for final composition
            GLuint m_originalSceneTexture = 0;
        };
    } // namespace postProcessing
} // namespace core