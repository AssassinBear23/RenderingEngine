#include "ObjectSystems/Components/Renderer.h"
#include "ObjectSystems/GameObject.h"
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

    void Scene::RemoveRootGameObject(const std::shared_ptr<GameObject>& go)
    {
        if (!go) return;
        m_roots.erase(std::remove(m_roots.begin(), m_roots.end(), go), m_roots.end());
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

    void Scene::RegisterRenderer(const std::shared_ptr<Renderer>& renderer)
    {
        if (!renderer) return;

        // Checks the list for the renderer, if it doesn't find it then find() returns the end iterator, which will then equal true and enter the if statement.
        if (std::find(m_renderers.begin(), m_renderers.end(), renderer) == m_renderers.end())
            m_renderers.push_back(renderer);
    }

    void Scene::UnregisterRenderer(const std::shared_ptr<Renderer>& renderer)
    {
        if (!renderer) return;
        m_renderers.erase(std::remove(m_renderers.begin(), m_renderers.end(), renderer), m_renderers.end());
    }

    void Scene::Render(const glm::mat4& view, const glm::mat4& projection)
    {
        glm::mat4 identity = glm::mat4(1.0f);

        for (const auto& renderer : m_renderers) {
            if (!renderer) continue;

            auto go = renderer->GetOwner();

            if (!go || !go->isEnabled || !renderer->isEnabled)
                continue;

            // Calculate world matrix
            glm::mat4 worldMatrix = CalculateWorldMatrix(go);
            glm::mat4 mvp = projection * view * worldMatrix;

            if (renderer->GetMaterial())
            {
                renderer->GetMaterial()->SetMat4("mvpMatrix", mvp);
                renderer->Render();
            }
        }
    }

    glm::mat4 Scene::CalculateWorldMatrix(const std::shared_ptr<GameObject>& go)
    {
        if (!go) return glm::mat4(1.0f);

        // Get local transform
        glm::mat4 localMatrix = glm::mat4(1.0f);
        if (go->transform) {
            localMatrix = go->transform->GetLocalMatrix();
        }

        // If has parent, multiply by parent's world matrix
        if (auto parent = go->GetParent().lock()) {
            return CalculateWorldMatrix(parent) * localMatrix;
        }

        return localMatrix;
    }
}
