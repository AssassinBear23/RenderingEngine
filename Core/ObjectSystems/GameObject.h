#pragma once
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <type_traits>
#include <algorithm>
#include "IComponent.h"

namespace core {

    // Forward declare for later; not used yet.
    class Transform;

    /// <summary>
    /// Scene node that owns components and manages parent/children.
    /// </summary>
    /// <remarks>
    /// Must keep:
    /// - Parent holds strong refs to children; child holds a weak ref to parent.
    /// - You can add a required Transform later. For now, this compiles without it.
    /// </remarks>
    class GameObject : public std::enable_shared_from_this<GameObject> {
    public:
        /// <summary>
        /// Create a GameObject. Name optional.
        /// </summary>
        explicit GameObject(std::string name = {});

        /// <summary>Unique ID (scene- or process-local).</summary>
        std::uint32_t GetId() const { return m_id; }

        /// <summary>Get/Set the display name.</summary>
        const std::string& GetName() const { return m_name; }
        void SetName(std::string n) { m_name = std::move(n); }

        /// <summary>Returns the parent (weak). Null/expired means root.</summary>
        std::weak_ptr<GameObject> GetParent() const { return m_parent; }

        /// <summary>Returns the list of children (strong).</summary>
        const std::vector<std::shared_ptr<GameObject>>& GetChildren() const { return m_children; }

        /// <summary>Reparents this object (removes from old, adds to new).</summary>
        void SetParent(std::shared_ptr<GameObject> newParent);

        /// <summary>Add a component of type T (constructed with args) and return it.</summary>
        template<typename T, typename... Args>
        std::shared_ptr<T> AddComponent(Args&&... args);

        /// <summary>Get the first component of type T, or null if none.</summary>
        template<typename T>
        std::shared_ptr<T> GetComponent() const;

        /// <summary>Get all components of type T.</summary>
        template<typename T>
        std::vector<std::shared_ptr<T>> GetComponents() const;

        /// <summary>Remove the first component of type T; returns true if removed.</summary>
        template<typename T>
        bool RemoveComponent();

        /// <summary>
        /// Returns this object's Transform if/when you add it as a component.
        /// For now, this returns nullptr.
        /// </summary>
        std::shared_ptr<Transform> Transform() const { return m_transform; }

        /// <summary>Enable/disable this GameObject (skips component updates when disabled).</summary>
        void SetActive(bool active) { m_active = active; }
        bool IsActive() const { return m_active; }

        /// <summary>Runs Start (once) and Update on enabled components.</summary>
        void Update();

        /// <summary>Calls Update() on this object and then recursively on children.</summary>
        void UpdateRecursive();

        /// <summary>Create a new child under this object and return it.</summary>
        std::shared_ptr<GameObject> CreateChild(std::string childName = {});

    private:
        std::uint32_t m_id = 0;
        std::string m_name;
        bool m_active = true;

        // Hierarchy
        std::weak_ptr<GameObject> m_parent;
        std::vector<std::shared_ptr<GameObject>> m_children;

        // Components
        std::vector<std::shared_ptr<IComponent>> m_components;

        // Placeholder; leave null until you actually implement/attach a Transform.
        std::shared_ptr<core::Transform> m_transform;

        // Internal component hooks
        void attachComponent(const std::shared_ptr<IComponent>& c);
        void detachComponent(const std::shared_ptr<IComponent>& c);
    };


    // ---------- Template implementations (keep in header or in an included .inl) ----------

    template<typename T, typename... Args>
    std::shared_ptr<T> GameObject::AddComponent(Args&&... args) {
        static_assert(std::is_base_of_v<IComponent, T>, "T must derive from IComponent");
        auto c = std::make_shared<T>(std::forward<Args>(args)...);
        attachComponent(c);
        return c;
    }

    template<typename T>
    std::shared_ptr<T> GameObject::GetComponent() const {
        for (auto& c : m_components) {
            if (auto cc = std::dynamic_pointer_cast<T>(c)) return cc;
        }
        return nullptr;
    }

    template<typename T>
    std::vector<std::shared_ptr<T>> GameObject::GetComponents() const {
        std::vector<std::shared_ptr<T>> out;
        out.reserve(m_components.size());
        for (auto& c : m_components) {
            if (auto cc = std::dynamic_pointer_cast<T>(c)) out.push_back(std::move(cc));
        }
        return out;
    }

    template<typename T>
    bool GameObject::RemoveComponent() {
        for (size_t i = 0; i < m_components.size(); ++i) {
            if (std::dynamic_pointer_cast<T>(m_components[i])) {
                detachComponent(m_components[i]);
                m_components.erase(m_components.begin() + i);
                return true;
            }
        }
        return false;
    }

} // namespace core
