#pragma warning(disable: 5246) // Suppress transitive include warning

#include "../frameBuffer.h"
#include "effects/postProcessingEffects.h"
#include "postProcessingEffectBase.h"
#include "postProcessingManager.h"
#include <algorithm>
#include <cstdio>
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
            FrameBuffer* sceneInput = &inputBuffer; // Store original scene input

            //printf("[POSTPROCESSMANAGER] Skipped %zu/%zu effects.\n", m_effects.size() - m_enabledEffects.size(), m_effects.size());

            if (m_enabledEffects.empty()) // If all effects were skipped, copy input to output directly using blit
            {
                inputBuffer.BindRead();
                outputBuffer.BindDraw();

                glReadBuffer(GL_COLOR_ATTACHMENT0);  
                glDrawBuffer(GL_COLOR_ATTACHMENT0);  

                glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return;
            }

            // Store the last processed output (for chaining effects)
            FrameBuffer* lastProcessedOutput = nullptr;

            for (size_t i = 0; i < m_enabledEffects.size(); i++)
            {
                auto& effect = m_enabledEffects[i];
                bool isLastEffect = (i == m_enabledEffects.size() - 1);
                
                if (isLastEffect)
                    currentOutput = &outputBuffer;

                // Determine which input to use
                FrameBuffer* effectInput;
                if (effect->RequiresSceneRender()) 
                    effectInput = sceneInput; // This effect needs the original scene input
                else
                    // This effect chains from the previous effect
                    // If this is the first effect, or the previous effect used scene input, use scene input
                    effectInput = (i == 0 || lastProcessedOutput == nullptr) ? sceneInput : lastProcessedOutput;

                printf("[POSTPROCESS MANAGER] Applying effect: %s (Passes: %d)\n== Parameters ==\nInput FBO name: %s\nOutput FBO name: %s\nWidth: %d\nHeight: %d\nRequires Scene Render: %s\n", 
                       effect->GetName().c_str(), 
                       effect->GetPassCount(),
                       effectInput->GetName().c_str(),
                       currentOutput->GetName().c_str(),
                       width,
                       height,
                       effect->RequiresSceneRender() ? "true" : "false"
                );

                effect->Apply(*effectInput, *currentOutput, width, height);
                
                // Update the last processed output for the next effect to potentially use
                lastProcessedOutput = currentOutput;
                
                // For next iteration: if current output is tempFBO, switch to inputBuffer for next output
                // Otherwise keep ping-ponging
                if (!isLastEffect)
                {
                    currentInput = currentOutput;
                    currentOutput = (currentOutput == &tempFBO) ? &inputBuffer : &tempFBO;
                }
            }
            printf("[POSTPROCESS MANAGER] Finished processing effects.\n\n");
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
