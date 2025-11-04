#include "GameObject.h"
#include "Component.h"

namespace core {

    GameObject::GameObject(std::string name) {
        SetName(std::move(name));
    }

    void GameObject::SetParent(std::shared_ptr<GameObject> newParent) {
        auto self = std::static_pointer_cast<GameObject>(shared_from_this());

        if (m_parent.lock() == newParent)
            return;

        // Remove from old parent
        if (auto old = m_parent.lock()) {
            auto& sibs = old->m_children;
            sibs.erase(std::remove(sibs.begin(), sibs.end(), self), sibs.end());
        }

        // Set new parent
        m_parent = newParent;

        // Add to new parent's children
        if (newParent) {
            auto& kids = newParent->m_children;
            if (std::find(kids.begin(), kids.end(), self) == kids.end()) {
                kids.push_back(self);
            }
        }
    }

    std::weak_ptr<GameObject> GameObject::GetParent() const { return m_parent; }
    const std::vector<std::shared_ptr<GameObject>>& GameObject::GetChildren() const { return m_children; }

    std::shared_ptr<GameObject> GameObject::CreateChild(std::string childName) {
        auto child = std::make_shared<GameObject>(std::move(childName));
        child->SetParent(std::static_pointer_cast<GameObject>(shared_from_this()));
        return child;
    }

    void GameObject::AddComponent(const std::shared_ptr<Component>& c) {
        if (!c) return;
        // avoid duplicates of the same instance
        if (std::find(m_components.begin(), m_components.end(), c) == m_components.end()) {
            c->OnAttach(std::static_pointer_cast<GameObject>(shared_from_this()));
            m_components.push_back(c);
        }
    }

    bool GameObject::RemoveComponent(const std::shared_ptr<Component>& c) {
        if (!c) return false;
        for (size_t i = 0; i < m_components.size(); ++i) {
            if (m_components[i].get() == c.get()) {
                m_components[i]->OnDetach();
                m_components.erase(m_components.begin() + i);
                return true;
            }
        }
        return false;
    }

    const std::vector<std::shared_ptr<Component>>& GameObject::GetComponents() const {
        return m_components;
    }

} // namespace core
