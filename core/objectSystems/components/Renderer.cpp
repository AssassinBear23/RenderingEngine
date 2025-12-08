#include "../../scene.h"
#include "../ComponentFactory.h"
#include "../GameObject.h"
#include "Renderer.h"
#include <core/ObjectSystems/component.h>
#include <glad/glad.h>
#include <imgui.h>
#include <memory>

namespace core
{
	REGISTER_COMPONENT(Renderer);

    void Renderer::Render(GLenum drawMode)
    {
        if (!m_material) return;

        m_material->Use();

        for (auto& mesh : m_meshes) {
            mesh.Render(drawMode);
        }
    }

    void Renderer::DrawGui()
    {
        ImGui::Text("Meshes: %zu", m_meshes.size());
		ImGui::Text("Material: %s", m_material ? "Set" : "Not Set");
    }

    void Renderer::OnAttach(std::weak_ptr<GameObject> owner)
    {
        Component::OnAttach(owner);
     
        if (auto go = owner.lock()) {
            if (auto scene = go->GetScene()) {
                scene->RegisterRenderer(std::static_pointer_cast<Renderer>(shared_from_this()));
                m_scene = scene;
            }
        }
    }

    void Renderer::OnDetach()
    {
        if (auto scene = m_scene.lock()) {
            scene->UnregisterRenderer(std::static_pointer_cast<Renderer>(shared_from_this()));
        }
        Component::OnDetach();
    }


} // namespace core