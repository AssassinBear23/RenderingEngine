#include "GameObject.h"
#include "Component.h"
#include "editor/editor.h"
#include "core/scene.h"

namespace core {

    GameObject::GameObject(std::string name) {
        SetName(std::move(name));
    }

    std::shared_ptr<GameObject> GameObject::Create(std::string name)
    {
        auto go = std::shared_ptr<GameObject>(new GameObject(std::move(name)));
        go->m_scene = editor::Editor::editorCtx.currentScene;
        go->Init();
        return go;
    }

    void GameObject::SetParent(std::shared_ptr<GameObject> newParent)
    {
        // Always use shared_from_this to get a shared_ptr to self, not shared_ptr<GameObject>(this).
        auto self = std::static_pointer_cast<GameObject>(shared_from_this());

        // Check if the passed parent is the same as current, if so, do nothing.
        if (m_parent.lock() == newParent) // .lock() converts a weak_ptr to shared_ptr. This allows you to compare.
            return;

        // Remove from old parent
        if (auto old = m_parent.lock()) {
            auto& sibs = old->m_children;
            sibs.erase(std::remove(sibs.begin(), sibs.end(), self), sibs.end());
        }
        else
        {
            // Was a root, remove from scene roots
            m_scene.lock()->RemoveRootGameObject(self);
        }

        // Set new parent
        m_parent = newParent;

        // Add to new parent's children 
        if (newParent) {
            newParent->AddChild(self);
        }
        else {
            editor::Editor::editorCtx.currentScene->AddRootGameObject(self);
        }
    }

    std::weak_ptr<GameObject> GameObject::GetParent() const { return m_parent; }

    void GameObject::AddChild(const std::shared_ptr<GameObject>& child)
    {
        if (!child) return;

        m_children.push_back(child);
    }

    void GameObject::RemoveChild(const std::shared_ptr<GameObject>& child)
    {
        if (!child) return;

        // Move elements that are not equal to the _Val child to the front, and returns an iterator to the new end, then use .erase to remove the "removed" elements.
        m_children.erase(std::remove(m_children.begin(), m_children.end(), child), m_children.end());
    }

    const std::vector<std::shared_ptr<GameObject>>& GameObject::GetChildren() const { return m_children; }


    const std::vector<std::shared_ptr<Component>>& GameObject::GetComponents() const {
        return m_components;
    }

} // namespace core
