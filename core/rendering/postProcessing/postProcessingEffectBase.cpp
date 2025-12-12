#include "../../material.h"
#include "../frameBuffer.h"
#include "postProcessingEffectBase.h"

namespace core
{
    namespace postProcessing
    {
        PostProcessingEffectBase::PostProcessingEffectBase(const std::string& name, std::shared_ptr<core::Material> material)
            : m_name(name), m_material(material)
        {}

        void PostProcessingEffectBase::Apply(GLuint inputTexture, core::FrameBuffer& outputFBO, const int width, const int height, unsigned int passIndex)
        {
            outputFBO.Bind();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, width, height);

            if (m_material)
            {
                m_material->Use();

                // Bind input texture to the material
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, inputTexture);

                m_material->SetInt("inputTexture", inputTexture);
            }
        }
    } // namespace postProcessing
} // namespace core