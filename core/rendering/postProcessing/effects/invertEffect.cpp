#include "../../../material.h"
#include "../../shader.h"
#include "invertEffect.h"
#include <imgui.h>


namespace core
{
    namespace postProcessing
    {
        InvertEffect::InvertEffect(std::weak_ptr<PostProcessingManager> manager)
            : PostProcessingEffectBase("InvertEffect", nullptr, manager)
        {
            m_shader = std::make_shared<Shader>("assets/shaders/postProcessing/postProcess.vert", "assets/shaders/postProcessing/invert.frag");
            m_material = std::make_shared<Material>(m_shader->ID);
        }

        void InvertEffect::Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height)
        {
            PostProcessingEffectBase::Apply(inputFBO, outputFBO, width, height);
        }

        void InvertEffect::DrawGui()
        {
            ImGui::Text("Inverts the colors of the input texture.");
        }
    } // namespace postProcessing
} // namespace core