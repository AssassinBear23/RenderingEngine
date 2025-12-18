#include "../../material.h"
#include "../frameBuffer.h"
#include "postProcessingEffectBase.h"
#include "postProcessingManager.h"


namespace core
{
    namespace postProcessing
    {
        PostProcessingEffectBase::PostProcessingEffectBase(const std::string& name, std::shared_ptr<core::Material> material, std::weak_ptr<PostProcessingManager> manager, bool requireSceneRender)
            : m_name(name), m_material(material), m_manager(manager), m_requireSceneRender(requireSceneRender)
        {
        }

        void PostProcessingEffectBase::Initialize()
        {
            isEnabled.SetOnChange([this](bool newValue) {
                if (auto manager = m_manager.lock())
                {
                    if (newValue)
                        manager->EnableEffect(shared_from_this());
                    else
                        manager->DisableEffect(shared_from_this());
                }
                });
        }

        void PostProcessingEffectBase::Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height)
        {
            outputFBO.Bind();

            CLEAR_BOUND(width, height);

            if (m_material)
            {
                m_material->SetTextureID("inputTexture", inputFBO.GetColorAttachment(), 0);
                m_material->Use();

                RenderQuad(width, height);
            }
        }

        static GLuint quadVAO = 0;
        static GLuint quadVBO = 0;

        void PostProcessingEffectBase::RenderQuad(const unsigned int width, const unsigned int height)
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