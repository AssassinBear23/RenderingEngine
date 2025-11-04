#include "Scene.h"

namespace core {

    Scene::Scene(std::string name) { SetName(std::move(name)); }

    std::shared_ptr<GameObject> Scene::CreateRoot(std::string name) {
        auto go = std::make_shared<GameObject>(std::move(name));
        m_roots.push_back(go);
        return go;
    }

    void Scene::AddRoot(const std::shared_ptr<GameObject>& go) {
        if (!go) return;
        if (std::find(m_roots.begin(), m_roots.end(), go) == m_roots.end())
            m_roots.push_back(go);
    }

    const std::vector<std::shared_ptr<GameObject>>& Scene::Roots() const { return m_roots; }

} // namespace core
