#include "../../scene.h"
#include "../componentFactory.h"
#include "../gameObject.h"
#include "Light.h"
#include "Renderer.h"

namespace core
{
    REGISTER_COMPONENT(Light);

    void Light::DrawGui()
    {
        ImGui::ColorPicker4("Light Color", (&color).ValuePtr());
    }


    void Light::UpdateRendererColor(glm::vec4 newColor)
    {
        if (auto renderer = m_renderer.lock()) {
            if (auto material = renderer->GetMaterial()) {
                material->SetVec4("lightColor", newColor);
            }
        }
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
            
            if (auto scene = go->GetScene())
            {
                m_scene = scene;
                scene->RegisterLight(std::static_pointer_cast<Light>(shared_from_this()));

                // Set callback to update renderer when color changes
                color.SetOnChange([this](glm::vec4 newColor) {
                    UpdateRendererColor(newColor);
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