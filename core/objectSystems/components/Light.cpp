#include "../../scene.h"
#include "../componentFactory.h"
#include "../gameObject.h"
#include "Light.h"
#include "Renderer.h"
#include <imgui.h>

namespace core
{
    REGISTER_COMPONENT(Light);

    // Used to draw Centered Text
    static void DrawCenteredText(const char* label, float rightOffset = 0.0f)
    {
        // Calculate text width and center position
        float textWidth = ImGui::CalcTextSize(label).x;
        float availableWidth = ImGui::GetContentRegionAvail().x - rightOffset;
        float textPosX = (availableWidth - textWidth) * 0.5f;

        if (textPosX > 0)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textPosX);

        ImGui::Text(label);
    }

    void Light::DrawGui()
    {
        // Use proxy system to make changes call the callback method.
        // Add HDR flag to allow values above 1.0
        ImGui::ColorPicker4("Light Color", glm::value_ptr(*&color), ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_Float);

        ImGui::Spacing();

        ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f);

        ImGui::Spacing();

        // Left arrow button
        float buttonWidth = ImGui::GetFrameHeight();
        if (ImGui::ArrowButton("##light_type_decrease", ImGuiDir_Left))
            --lightType;

        ImGui::SameLine();

        // Draw centered text with right arrow offset
        std::string lightTypeLabel = std::string("Light Type: ") + core::ToString(lightType.Get());
        DrawCenteredText(lightTypeLabel.c_str(), buttonWidth);

        // Align right arrow to the right
        ImGui::SameLine();
        float rightArrowPosX = ImGui::GetWindowWidth() - buttonWidth - ImGui::GetStyle().WindowPadding.x;
        ImGui::SetCursorPosX(rightArrowPosX);

        if (ImGui::ArrowButton("##light_type_increase", ImGuiDir_Right))
            ++lightType;
    }

    void Light::UpdateRendererColor(glm::vec4 newColor)
    {
        if (auto renderer = m_renderer.lock())
            if (auto material = renderer->GetMaterial())
                material->SetVec4("lightColor", newColor);
    }

    void Light::UpdateRendererIntensity(float newIntensity)
    {
        if (auto renderer = m_renderer.lock())
            if (auto material = renderer->GetMaterial())
                material->SetFloat("intensity", newIntensity);
    }

    void Light::OnAttach(std::weak_ptr<GameObject> owner)
    {
        Component::OnAttach(owner);
        if (auto go = owner.lock())
        {
            // Cache the renderer component
            m_renderer = go->GetComponent<Renderer>();

            // Set initial color
            UpdateRendererColor(color.Get());
            UpdateRendererIntensity(intensity.Get());

            if (auto scene = go->GetScene())
            {
                m_scene = scene;
                scene->RegisterLight(std::static_pointer_cast<Light>(shared_from_this()));

                // Set callback to update renderer when color changes
                color.SetOnChange([this](glm::vec4 newColor) {
                    UpdateRendererColor(newColor);
                    });
                intensity.SetOnChange([this](float newIntensity) {
                    UpdateRendererIntensity(newIntensity);
                    });
            }
        }
    }

    void Light::OnDetach()
    {
        if (auto scene = m_scene.lock())
        {
            scene->UnregisterLight(std::static_pointer_cast<Light>(shared_from_this()));
        }
    }
} // namespace core