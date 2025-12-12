#include "../../../material.h"
#include "../../frameBuffer.h"
#include "../../shader.h"
#include "bloomEffect.h"
#include <imgui.h>


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

        void BloomEffect::Apply(GLuint inputTexture, core::FrameBuffer& outputFBO, const int width, const int height, unsigned int passIndex)
        {
            outputFBO.Bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, width, height);

            std::shared_ptr<core::Material> material;

            // First pass: bright pass (threshold)
            if (passIndex == 0)
            {
                // Store original scene texture for final composition
                m_originalSceneTexture = inputTexture;
                
                material = m_brightPassMaterial;
                material->SetFloat("threshold", m_bloomThreshold);
                
                material->Use();
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, inputTexture);
                material->SetInt("inputTexture", 0);
            }
            // Blur passes
            else
            {
                material = m_blurMaterial;
                bool horizontal = (passIndex % 2) == 1;
                material->SetBool("horizontal", horizontal);
                
                // Check if this is the final pass
                bool isFinalPass = (passIndex == GetPassCount() - 1) && (m_debugMode == BloomDebugMode::None);
                material->SetBool("isFinalPass", isFinalPass);
                
                if (isFinalPass)
                {
                    material->SetFloat("intensity", m_intensity);
                }
                
                material->Use();
                
                // Bind blurred input
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, inputTexture);
                material->SetInt("inputTexture", 0);
                
                // If final pass, also bind original scene
                if (isFinalPass)
                {
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, m_originalSceneTexture);
                    material->SetInt("sceneTexture", 1);
                }
            }
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