#include "../../../material.h"
#include "../../shader.h"
#include "../postProcessingManager.h"
#include "fogEffect.h"
#include <imgui.h>

namespace core
{
    namespace postProcessing
    {
        FogEffect::FogEffect(std::weak_ptr<PostProcessingManager> manager)
            : PostProcessingEffectBase("FogEffect", nullptr, manager, false)
        {
            m_shader = std::make_shared<Shader>("assets/shaders/postProcessing/postProcess.vert", "assets/shaders/postProcessing/fog.frag");
            m_material = std::make_shared<Material>(m_shader->ID);
        }

        void FogEffect::Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height)
        {
            outputFBO.Bind();
            CLEAR_BOUND(width, height);

            // Get depth from the original scene render through the manager
            if (auto manager = m_manager.lock())
            {
                GLuint depthTexture = manager->GetSceneDepthTexture();
                
                if (m_material)
                {
                    // Set color texture from previous effect
                    m_material->SetTextureID("inputTexture", inputFBO.GetColorAttachment(), 0);
                    
                    // Set depth texture from original scene
                    m_material->SetTextureID("depthTexture", depthTexture, 1);
                    
                    // Set fog parameters
                    m_material->SetVec3("fogColor", m_fogColor.Get());
                    m_material->SetFloat("fogDensity", m_fogDensity.Get());
                    m_material->SetFloat("fogStart", m_fogStart.Get());
                    m_material->SetFloat("fogEnd", m_fogEnd.Get());
                    m_material->SetInt("fogMode", static_cast<int>(m_fogMode.Get()));
                    m_material->SetInt("debugMode", static_cast<int>(m_debugMode.Get()));
                    m_material->SetFloat("nearPlane", m_nearPlane);
                    m_material->SetFloat("farPlane", m_farPlane);
                    
                    m_material->Use();
                    RenderQuad(width, height);
                }
            }
        }
        
        void FogEffect::DrawGui()
        {
            ImGui::PushID(this);

            if (ImGui::CollapsingHeader("Fog Settings", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();

                // Debug mode
                const char* debugModes[] = { "Normal", "Raw Depth", "Linear Depth", "Fog Factor" };
                int currentDebugMode = static_cast<int>(m_debugMode.Get());
                if (ImGui::Combo("Debug Mode", &currentDebugMode, debugModes, IM_ARRAYSIZE(debugModes)))
                {
                    m_debugMode = static_cast<FogDebugMode>(currentDebugMode);
                }

                if (m_debugMode != FogDebugMode::Normal)
                {
                    ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Debug mode active!");
                }

                ImGui::Separator();

                // Fog mode selector
                const char* fogModes[] = { "Linear", "Exponential", "Exponential Squared" };
                int currentMode = static_cast<int>(m_fogMode.Get());
                if (ImGui::Combo("Fog Mode", &currentMode, fogModes, IM_ARRAYSIZE(fogModes)))
                {
                    m_fogMode = static_cast<FogMode>(currentMode);
                }

                ImGui::Separator();

                // Fog color
                float color[3] = { m_fogColor.Get().x, m_fogColor.Get().y, m_fogColor.Get().z };
                if (ImGui::ColorEdit3("Fog Color", color))
                {
                    m_fogColor = glm::vec3(color[0], color[1], color[2]);
                }

                ImGui::Separator();

                // Mode-specific parameters
                if (m_fogMode == FogMode::Linear)
                {
                    float start = m_fogStart.Get();
                    if (ImGui::SliderFloat("Fog Start", &start, 0.1f, 500.0f, "%.1f"))
                    {
                        m_fogStart = start;
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Distance where fog starts");

                    float end = m_fogEnd.Get();
                    if (ImGui::SliderFloat("Fog End", &end, 1.0f, 1000.0f, "%.1f"))
                    {
                        m_fogEnd = end;
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Distance where fog is completely opaque");
                }
                else // Exponential modes
                {
                    float density = m_fogDensity.Get();
                    if (ImGui::SliderFloat("Fog Density", &density, 0.001f, 0.5f, "%.3f"))
                    {
                        m_fogDensity = density;
                    }
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("How quickly fog accumulates with distance");
                }

                ImGui::Separator();

                // Camera parameters
                ImGui::Text("Camera Parameters");
                ImGui::SliderFloat("Near Plane", &m_nearPlane, 0.01f, 10.0f, "%.2f");
                ImGui::SliderFloat("Far Plane", &m_farPlane, 10.0f, 10000.0f, "%.1f");
                
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Must match your camera's near/far planes!");

                ImGui::Unindent();
            }

            ImGui::PopID();
        }
    }
}
