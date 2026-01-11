#include "../../../material.h"
#include "../../frameBuffer.h"
#include "../../shader.h"
#include "bloomEffect.h"
#include "../../../scene.h"
#include "../../../../editor/Editor.h"
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

            m_bloomThreshold.SetOnChange([this](float newThreshold)
                {
                    if (auto currentScene = editor::Editor::editorCtx.currentScene)
                        currentScene->SetBloomThreshold(newThreshold);
                });
        }

        int BloomEffect::GetPassCount() const
        {
            return m_blurAmount * 2;
        }

        void BloomEffect::Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height)
        {
            std::shared_ptr<core::Material> material;

            GLuint thresholdTexture = inputFBO.GetColorAttachment(1);
            
            tempFBO_1.Resize(width, height);
            tempFBO_2.Resize(width, height);

            if (m_debugMode == BloomDebugMode::ThresholdOnly)
            {
                // Blit the second color attachment (bright pixels) directly to output
                outputFBO.BindDraw();
                inputFBO.BindRead();

                glReadBuffer(GL_COLOR_ATTACHMENT1);
                glDrawBuffer(GL_COLOR_ATTACHMENT0);

                glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                
                glReadBuffer(GL_COLOR_ATTACHMENT0);
                return;
            }

            FrameBuffer* targetFBO = nullptr;
            FrameBuffer* sourceFBO = nullptr;
            FrameBuffer* lastFBO = nullptr;

            tempFBO_1.BindAndClear(width, height);
            tempFBO_2.BindAndClear(width, height);
            tempFBO_2.Unbind();

            for (int passIndex = 0; passIndex < GetPassCount(); passIndex++)
            {
                // Set the target and source FBO, as well as determining if doing a horizontal pass.
                bool horizontal = passIndex % 2 == 0;
                targetFBO = horizontal ? &tempFBO_1 : &tempFBO_2;
                sourceFBO = (targetFBO == &tempFBO_1) ? &tempFBO_2 : &tempFBO_1;

                bool returnAfter = m_debugMode == BloomDebugMode::BlurOnly && passIndex + 1 == GetPassCount();

                if (returnAfter)
                    outputFBO.BindAndClear(width, height);
                else 
                    targetFBO->BindAndClear(width, height);

                // Set Material
                material = m_blurMaterial;
                material->SetBool("horizontal", horizontal);
                material->SetFloat("intensity", m_intensity);

                // First pass uses the threshold texture from MRT, subsequent passes use temp buffers
                if (passIndex == 0)
                    material->SetTextureID("inputTexture", thresholdTexture, 0);
                else
                    material->SetTextureID("inputTexture", sourceFBO->GetColorAttachment(), 0);

                material->Use();


                RenderQuad(width, height);

                if (returnAfter)
                    return;

                lastFBO = targetFBO;
            }

            outputFBO.BindAndClear(width, height);

            material = m_compositeMaterial;
            material->SetTextureID("sceneTexture", inputFBO.GetColorAttachment(), 0);
            material->SetTextureID("bloomTexture", lastFBO->GetColorAttachment(), 1);
            material->Use();

            CLEAR_BOUND(width, height);
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
                ImGui::SliderFloat("Threshold", &m_bloomThreshold, 0.0f, 20.0f, "%.2f");
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
                    ImGui::Text("(Show the BrightPixels texture directly to screen)");
                }
                else if (m_debugMode == BloomDebugMode::BlurOnly)
                {
                    ImGui::Text("(Dont Composite with the final scene. Shows only the blurring of the BrightPixels)");
                }

                ImGui::Unindent();
            }

            ImGui::PopID();
        }
    } // namespace postProcessing
} // namespace core