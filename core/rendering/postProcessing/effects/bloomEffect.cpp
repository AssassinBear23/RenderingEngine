#include "../../../material.h"
#include "../../frameBuffer.h"
#include "../../shader.h"
#include "bloomEffect.h"
#include <glad/glad.h>
#include <imgui.h>


namespace core
{
    namespace postProcessing
    {
        BloomEffect::BloomEffect(std::weak_ptr<PostProcessingManager> manager)
            : PostProcessingEffectBase("BloomEffect", nullptr, manager, true)
        {
            m_blurShader = std::make_shared<Shader>("assets/shaders/postProcessing/postProcess.vert", "assets/shaders/postProcessing/bloomBlur.frag");
            m_compositeShader = std::make_shared<Shader>("assets/shaders/postProcessing/postProcess.vert", "assets/shaders/postProcessing/composite.frag");
            m_blurMaterial = std::make_shared<Material>(m_blurShader->ID);
            m_compositeMaterial = std::make_shared<Material>(m_compositeShader->ID);

            tempFBO_1 = FrameBuffer("postProcessFBO_1", FrameBufferSpecifications{ 100, 100, AttachmentType::COLOR_ONLY });
            tempFBO_2 = FrameBuffer("postProcessFBO_2", FrameBufferSpecifications{ 100, 100, AttachmentType::COLOR_ONLY });
        }

        int BloomEffect::GetPassCount() const
        {
            // In debug modes, we may want to see only certain passes
            switch (m_debugMode)
            {
            case BloomDebugMode::ThresholdOnly:
                return 1; // Only the bright pass
            case BloomDebugMode::BlurOnly:
                return 1 + (m_blurAmount * 2); // Threshold + blur passes (no combine)
            case BloomDebugMode::None:
            default:
                return 1 + (m_blurAmount * 2); // Threshold + blur passes (last blur combines)
            }
        }

        void BloomEffect::Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height)
        {
            tempFBO_1.Bind();

            if (m_debugMode == BloomDebugMode::ThresholdOnly)
                outputFBO.Bind();

            CLEAR_BOUND(width, height);

            std::shared_ptr<core::Material> material;

            //// First pass: bright pass (threshold)
            material = m_brightPassMaterial;
            material->SetFloat("threshold", m_bloomThreshold);

            //// Bind input texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, inputFBO.GetColorAttachment());
            material->SetInt("inputTexture", 0);

            material->Use();

            RenderQuad(width, height);  // Render threshold to tempFBO_2
            if (m_debugMode == BloomDebugMode::ThresholdOnly)
                return;

            FrameBuffer* lastFBO = nullptr;

            //// Blur passes
            for (int passIndex = 0; passIndex < GetPassCount() * 2; ++passIndex)
            {
                bool horizontal = (passIndex % 2) == 0;
                FrameBuffer* targetFBO = horizontal ? &tempFBO_1 : &tempFBO_2;
                FrameBuffer* sourceFBO = horizontal ? &tempFBO_2 : &tempFBO_1;

                targetFBO->Bind();
                if (m_debugMode == BloomDebugMode::BlurOnly && passIndex + 1 == GetPassCount() * 2)
                    outputFBO.Bind();

                CLEAR_BOUND(width, height);

                material = m_blurMaterial;
                material->SetBool("horizontal", horizontal);

                //// Bind blurred input
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, sourceFBO->GetColorAttachment());
                material->SetInt("inputTexture", 0);

                material->Use();
                RenderQuad(width, height);

                lastFBO = targetFBO;

                if (m_debugMode == BloomDebugMode::BlurOnly)
                    return;
            }

            material = m_compositeMaterial;

            // Bind the original scene texture
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, inputFBO.GetColorAttachment());
            material->SetInt("sceneTexture", 0);

            // Bind the blurred bloom texture
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, lastFBO->GetColorAttachment());
            material->SetInt("bloomTexture", 1);

            outputFBO.Bind();
            CLEAR_BOUND(width, height);

            material->Use();
            RenderQuad(width, height);
        }

        void BloomEffect::DrawGui()
        {
            ImGui::PushID(this);

            if (ImGui::CollapsingHeader("Bloom Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();

                // Debug mode selector
                const char* debugModes[] = { "None (Normal)", "Threshold Only", "Blur Only" };
                int currentMode = static_cast<int>(m_debugMode);
                if (ImGui::Combo("Debug Mode", &currentMode, debugModes, IM_ARRAYSIZE(debugModes)))
                {
                    m_debugMode = static_cast<BloomDebugMode>(currentMode);
                }

                if (m_debugMode != BloomDebugMode::None)
                {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Debug mode active!");

                    if (m_debugMode == BloomDebugMode::ThresholdOnly)
                    {
                        ImGui::TextWrapped("Showing only pixels above threshold (bright pass)");
                    }
                    else if (m_debugMode == BloomDebugMode::BlurOnly)
                    {
                        ImGui::TextWrapped("Showing blurred bright areas without combining with scene");
                    }
                }

                ImGui::Separator();

                // Bloom parameters
                ImGui::SliderFloat("Threshold", &m_bloomThreshold, 0.0f, 2.0f, "%.2f");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Pixels brighter than this value will bloom");
                }

                ImGui::SliderFloat("Intensity", &m_intensity, 0.0f, 5.0f, "%.2f");
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("Multiplier for the bloom effect strength");
                }

                ImGui::SliderInt("Blur Passes", &m_blurAmount, 1, 10);
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip("More passes = smoother blur but slower");
                }

                // Info display
                ImGui::Separator();
                ImGui::Text("Total Passes: %d", GetPassCount());
                if (m_debugMode == BloomDebugMode::ThresholdOnly)
                {
                    ImGui::Text("(1 threshold pass only - debug mode)");
                }
                else if (m_debugMode == BloomDebugMode::BlurOnly)
                {
                    ImGui::Text("(1 threshold + %d blur passes - debug mode)", m_blurAmount * 2);
                }
                else
                {
                    ImGui::Text("(1 threshold + %d blur with final combine)", m_blurAmount * 2);
                }

                ImGui::Unindent();
            }

            ImGui::PopID();
        }
    } // namespace postProcessing
} // namespace core