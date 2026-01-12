#include "GameObject.h"
#include "Component.h"
#include "editor/editor.h"
#include "core/scene.h"
#include "ComponentFactory.h"

namespace core {
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

    GameObject::GameObject(std::string name) {
        SetName(std::move(name));
    }

    void GameObject::SetChildrenEnabledState(bool enabled)
    {
        for (const auto& child : m_children) {
            child->isEnabled = enabled;
        }
    }

    void GameObject::OnEnabledChanged(bool newValue)
    {
        // Call base implementation
        Object::OnEnabledChanged(newValue);
        // Propagate to children
        SetChildrenEnabledState(newValue);
    }

    void GameObject::Serialize(nlohmann::json& out) const
    {
        // Base info
        Object::Serialize(out);

        // Components
        nlohmann::json compsArray = nlohmann::json::array();
        for (const auto& comp : m_components) {
            nlohmann::json compJson;
            compJson["type"] = comp->GetTypeName();
            comp->Serialize(compJson);
            compsArray.push_back(compJson);
        }

        out["components"] = compsArray;

        // Children (recursive)
        nlohmann::json childrenArray = nlohmann::json::array();
        for (const auto& child : m_children) {
            nlohmann::json childJson;
            child->Serialize(childJson);
            childrenArray.push_back(std::move(childJson));
        }

        out["children"] = childrenArray;
    }

    void GameObject::Deserialize(const nlohmann::json& in)
    {
        // Base info
        Object::Deserialize(in);

        // Components
        if (in.contains("components") && in["components"].is_array()) {
            for (const auto& compJson : in["components"]) {
                const std::string type = compJson.value("type", "");

                if (type.empty()) continue;

                if (type == "Transform" && transform)
                    transform->Deserialize(compJson);
                else {
                    auto comp = ComponentFactory::Create(type);
                    if (comp) {
                        comp->Deserialize(compJson);
                        comp->OnAttach(std::static_pointer_cast<GameObject>(shared_from_this()));
                        m_components.push_back(comp);
                    }
                }
            }
        }

        // Children (recursive)
        if (in.contains("children") && in["children"].is_array()) {
            for (const auto& childJson : in["children"]) {
                auto childGO = GameObject::Create();
                childGO->Deserialize(childJson);
                childGO->SetParent(std::static_pointer_cast<GameObject>(shared_from_this()));
            }
        }
    }
} // namespace core
