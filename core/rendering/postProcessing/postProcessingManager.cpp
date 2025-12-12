#include "../frameBuffer.h"
#include "postProcessingEffectBase.h"
#include "postProcessingManager.h"
#include <algorithm>
#include <memory>
#include <utility>

namespace core
{
    namespace postProcessing
    {
        void PostProcessingManager::ProcessStack(FrameBuffer& inputBuffer, FrameBuffer& outputBuffer, const unsigned int width, const unsigned int height)
        {
            FrameBuffer tempFBO_1(FrameBufferSpecifications{ width, height, AttachmentType::COLOR_ONLY, GL_RGBA16F, GL_DEPTH_COMPONENT });
            FrameBuffer tempFBO_2(FrameBufferSpecifications{ width, height, AttachmentType::COLOR_ONLY, GL_RGBA16F, GL_DEPTH_COMPONENT });

            GLuint currentInput = inputBuffer.GetColorAttachment();

            FrameBuffer* currentOutput = &tempFBO_1;
            FrameBuffer* nextOutput = &tempFBO_2;
            for (auto& effect : m_effects)
            {
                if (!effect->isEnabled) continue; // Skip disabled effects

                int passCount = effect->GetPassCount();

                for (int pass = 0; pass < passCount; ++pass)
                {
                    bool isLastPass = (pass == passCount - 1 && effect == m_effects.back());
                    FrameBuffer& targetFBO = isLastPass ? outputBuffer : *currentOutput;

                    effect->Apply(currentInput, targetFBO, width, height, pass);
                    RenderQuad(width, height);
                    currentInput = targetFBO.GetColorAttachment();
                    std::swap(currentOutput, nextOutput); // Ping-Pong the buffers
                }
            }
        }

        bool PostProcessingManager::AddEffect(const std::shared_ptr<PostProcessingEffectBase> effect)
        {
            if (!effect) return false;

            if (std::find(m_effects.begin(), m_effects.end(), effect) == m_effects.end())
            {
                m_effects.push_back(effect);
                return true;
            }
            return false;
        }

        bool PostProcessingManager::RemoveEffect(const std::shared_ptr<PostProcessingEffectBase> effect)
        {
            if (!effect) return false;

            auto it = std::find(m_effects.begin(), m_effects.end(), effect);
            if (it != m_effects.end())
            {
                m_effects.erase(it);
                return true;
            }
            return false;
        }

        static GLuint quadVAO = 0;
        static GLuint quadVBO = 0;

        void PostProcessingManager::RenderQuad(const unsigned int width, const unsigned int height)
        {
            if (quadVAO == 0)
            {
                float quadVertices[] = {
                    // positions   // texCoords
                    -1.0f,  1.0f,  0.0f, 1.0f,
                    -1.0f, -1.0f,  0.0f, 0.0f,
                     1.0f, -1.0f,  1.0f, 0.0f,
                    -1.0f,  1.0f,  0.0f, 1.0f,
                     1.0f, -1.0f,  1.0f, 0.0f,
                     1.0f,  1.0f,  1.0f, 1.0f
                };
                glGenVertexArrays(1, &quadVAO);
                glGenBuffers(1, &quadVBO);
                glBindVertexArray(quadVAO);
                glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
                glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                glEnableVertexAttribArray(0);
                glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
                glEnableVertexAttribArray(1);
                glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
            }

            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glBindVertexArray(0);
        }
    } // namespace postProcessing
} // namespace core
