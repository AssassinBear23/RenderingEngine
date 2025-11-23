#include "Scene.h"

namespace core {

    Scene::Scene(std::string name) { SetName(std::move(name)); }

    void Scene::SetName(std::string name) { m_name = std::move(name); }
    const std::string& Scene::GetName() const { return m_name; }

    void Scene::AddRootGameObject(const std::shared_ptr<GameObject>& go) {
        if (!go) return;
        if (std::find(m_roots.begin(), m_roots.end(), go) == m_roots.end())
            m_roots.push_back(go);
    }

    std::shared_ptr<GameObject> Scene::CreateObject(const std::string& name, const std::shared_ptr<core::GameObject> parent)
    {
        auto newObject = GameObject::Create(name);
        if (parent) {
            newObject->SetParent(parent);
        }
        else {
            AddRootGameObject(newObject);
        }
        return newObject;
    }

    const std::vector<std::shared_ptr<GameObject>>& Scene::Roots() const { return m_roots; }

    void Scene::Render(const glm::mat4& view, const glm::mat4& projection)
    {
        glm::mat4 identity = glm::mat4(1.0f);
        for (const auto& root : m_roots) {
            RenderGameObject(root, identity, view, projection);
        }
    }

    void Scene::RenderGameObject(const std::shared_ptr<GameObject>& go,
        const glm::mat4& parentMatrix,
        const glm::mat4& view,
        const glm::mat4& projection)
    {
        if (!go) return; // If game object is null, return early.

        // Get Components
        std::shared_ptr<core::Transform> transform = nullptr;
        std::shared_ptr<core::Renderer> renderer = nullptr;
        for (const auto& comp : go->GetComponents()) {
            if (!transform) {
                transform = std::dynamic_pointer_cast<core::Transform>(comp);
            }
            if (!renderer) {
                renderer = std::dynamic_pointer_cast<core::Renderer>(comp);
            }
            if (transform && renderer) break; // Found both components
        }

        // Calculate world matrix
        //if ()

    }
}
