#include "Scene.h"

namespace core {

    Scene::Scene(std::string name) { SetName(std::move(name)); }

    void Scene::AddRootGameObject(const std::shared_ptr<GameObject>& go) {
        if (!go) return;
        if (std::find(m_roots.begin(), m_roots.end(), go) == m_roots.end())
            m_roots.push_back(go);
    }

    std::shared_ptr<GameObject> Scene::CreateObject(const std::string& name, const std::shared_ptr<core::GameObject> parent)
    {
        auto newObject = std::make_shared<GameObject>(name);
        if (parent) {
            newObject->SetParent(parent);
        } else {
            AddRootGameObject(newObject);
        }
        return newObject;
    }

    /*void Scene::Draw()
    {
        for (const auto& object : m_roots)
        {
            if(object->GetComponents())
        }
    }*/

    const std::vector<std::shared_ptr<GameObject>>& Scene::Roots() const { return m_roots; }
}
