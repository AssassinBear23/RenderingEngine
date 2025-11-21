#pragma once
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include "Object.h"

namespace core {

    class Component; // forward

    /// <summary>
    /// A scene node that can have children and components.
    /// </summary>
    /// <remarks>
    /// Must keep:
    /// - Parent holds strong refs to children; child holds a weak ref to parent.
    /// - Components are stored as shared_ptr and detached by reference.
    /// </remarks>
    class GameObject : public Object {
    public:
        /// <summary>
        /// Construct a GameObject. Name optional.
        /// </summary>
        explicit GameObject(std::string name = {});

        /// <summary>
        /// Set this object's parent (handles both sides of the relation)
        /// <para>
        /// Pass nullptr to make it a root.
        /// </para>
        /// </summary>
        void SetParent(std::shared_ptr<GameObject> newParent);

        /// <summary>
        /// Returns the parent (weak). Null/expired means root.
        /// </summary>
        std::weak_ptr<GameObject> GetParent() const;
        /// <summary>
        /// Adds a child to this objects child list.
        /// </summary>
        /// <param name="child"></param>
        void AddChild(const std::shared_ptr<GameObject>& child);
        /// <summary>
        /// Removes a child from this objects child list.
        /// </summary>
        /// <param name="child"></param>
        void RemoveChild(const std::shared_ptr<GameObject>& child);
        /// <summary>
        /// Returns the children (strong).
        /// </summary>
        const std::vector<std::shared_ptr<GameObject>>& GetChildren() const;

        /// <summary>
        /// Create and return a child under this object.
        /// </summary>
        std::shared_ptr<GameObject> CreateChild(std::string childName = {});

        /// <summary>
        /// Add a component by reference (exact instance).
        /// Calls component->OnAttach(this).
        /// </summary>
        void AddComponent(const std::shared_ptr<Component>& c);

        /// <summary>
        /// Remove a component by reference (exact instance). Returns true if removed.
        /// Calls component->OnDetach() before erasing.
        /// </summary>
        bool RemoveComponent(const std::shared_ptr<Component>& c);

        /// <summary>
        /// Returns all components attached to this GameObject.
        /// </summary>
        const std::vector<std::shared_ptr<Component>>& GetComponents() const;
    private:
        std::weak_ptr<GameObject> m_parent;
        std::vector<std::shared_ptr<GameObject>> m_children;
        std::vector<std::shared_ptr<Component>>   m_components;
    };

} // namespace core
