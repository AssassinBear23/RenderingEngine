#pragma warning(disable: 5246) // Suppress transitive include warning

#include "../frameBuffer.h"
#include "effects/postProcessingEffects.h"
#include "postProcessingEffectBase.h"
#include "postProcessingManager.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>

namespace core
{
    namespace postProcessing
    {
        PostProcessingManager::PostProcessingManager()
        {
            tempFBO = FrameBuffer("postProcessFBO", FrameBufferSpecifications{ 100, 100, AttachmentType::COLOR_ONLY, GL_RGBA16F, GL_DEPTH_COMPONENT });
        }

        void PostProcessingManager::ProcessStack(FrameBuffer& inputBuffer, FrameBuffer& outputBuffer, const unsigned int width, const unsigned int height)
        {
            tempFBO.Resize(width, height);

            FrameBuffer* currentInput = &inputBuffer;
            FrameBuffer* currentOutput = &tempFBO;

            //printf("[POSTPROCESSMANAGER] Skipped %zu/%zu effects.\n", m_effects.size() - m_enabledEffects.size(), m_effects.size());

            if (m_enabledEffects.empty()) // If all effects were skipped, copy input to output directly using blit
            {
                inputBuffer.BindRead();
                outputBuffer.BindDraw();
                glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return;
            }

            for (auto& effect : m_enabledEffects)
            {
                bool isLastEffect = (effect == m_enabledEffects.back());
                if (isLastEffect)
                    currentOutput = &outputBuffer;

                printf("[POSTPROCESS MANAGER] Applying effect: %s (Passes: %d)\n== Parameters ==\nInput FBO name: %s\nOutput FBO name: %s\nWidth: %d\nHeight: %d\n", 
                       effect->GetName().c_str(), 
                       effect->GetPassCount(),
                       currentInput->GetName().c_str(),
                       currentOutput->GetName().c_str(),
                       width,
                       height
                );
                effect->Apply(*currentInput, *currentOutput, width, height);
                currentInput = currentOutput;
            }
            std::cout << "[POSTPROCESS MANAGER] Finished processing effects." << std::endl << std::endl;
        }

        bool PostProcessingManager::AddEffect(const std::shared_ptr<PostProcessingEffectBase> effect)
        {
            if (!effect) return false;

            if (std::find(m_effects.begin(), m_effects.end(), effect) == m_effects.end())
            {
                effect->Initialize();
                m_effects.push_back(effect);
                return true;
            }
            return false;
        }

        void PostProcessingManager::EnableEffect(const std::shared_ptr<PostProcessingEffectBase> effect)
        {
            if (!effect) return;

            if (std::find(m_enabledEffects.begin(), m_enabledEffects.end(), effect) == m_enabledEffects.end())
                m_enabledEffects.push_back(effect);

            SortEnabledEffects();
        }

        void PostProcessingManager::DisableEffect(const std::shared_ptr<PostProcessingEffectBase> effect)
        {
            if (!effect) return;
            m_enabledEffects.erase(std::remove(m_enabledEffects.begin(), m_enabledEffects.end(), effect), m_enabledEffects.end());
        }

        void PostProcessingManager::Initialize()
        {
            AddEffect(std::make_shared<postProcessing::BloomEffect>(weak_from_this()));
            AddEffect(std::make_shared<postProcessing::InvertEffect>(weak_from_this()));
        }

        void PostProcessingManager::SortEnabledEffects()
        {
            std::sort(m_enabledEffects.begin(), m_enabledEffects.end(), [this](const std::shared_ptr<PostProcessingEffectBase>& a, const std::shared_ptr<PostProcessingEffectBase>& b) {
                auto itA = std::find(m_effects.begin(), m_effects.end(), a);
                auto itB = std::find(m_effects.begin(), m_effects.end(), b);
                auto indexA = std::distance(m_effects.begin(), itA);
                auto indexB = std::distance(m_effects.begin(), itB);
                return indexA < indexB;
                      });
        }
    } // namespace postProcessing
} // namespace core
