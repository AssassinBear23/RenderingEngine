#include "bloomEffect.h"
#include "../../../material.h"
#include "../../frameBuffer.h"
#include "../../shader.h"

namespace core
{
    namespace postProcessing
    {
        BloomEffect::BloomEffect()
            : PostProcessingEffectBase("BloomEffect", nullptr)
        {
            core::Shader brightPassShader("assets/shaders/postProcessing/postProcess.vert", "assets/shaders/postProcessing/bloomBrightPass.frag");
            core::Shader blurShader("assets/shaders/postProcessing/postProcess.vert", "assets/shaders/postProcessing/bloomBlur.frag");

            m_brightPassMaterial = std::make_shared<core::Material>(brightPassShader.ID);
            m_blurMaterial = std::make_shared<core::Material>(blurShader.ID);
        }

        void BloomEffect::Apply(GLuint inputTexture, core::FrameBuffer& outputFBO, const int width, const int height, unsigned int passIndex)
        {
            outputFBO.Bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, width, height);

            std::shared_ptr<core::Material> material;

            if (passIndex == 0)
            {
                material = m_brightPassMaterial;
                material->SetFloat("threshold", m_blurThreshold);
            }
            else
            {
                material = m_blurMaterial;
                bool horizontal = (passIndex % 2) == 1;
                material->SetBool("horizontal", horizontal);
            }

            material->Use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, inputTexture);
            material->SetInt("inputTexture", 0);
        }

        void BloomEffect::DrawGui()
        {
            // Implement ImGui controls for bloom effect parameters here
        }
    } // namespace postProcessing
} // namespace core