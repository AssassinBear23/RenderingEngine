#pragma once
#include <memory>
#include "Object.h"

namespace core {

    class GameObject; // forward

    /// <summary>
    /// Base class for components that attach to a GameObject.
    /// Holds a weak back-reference to the owner.
    /// </summary>
    /// <remarks>
    /// Must keep:
    /// - Components are owned by a GameObject via std::shared_ptr.
    /// - Back-reference is weak_ptr to avoid cycles.
    /// </remarks>
    class Component : public Object {
    public:
        virtual ~Component() = default;

        virtual std::string GetTypeName() const { return "Component"; }

        /// <summary>
        /// Called by GameObject when this component is attached.
        /// Stores the back-reference to the owner.
        /// </summary>
        virtual void OnAttach(std::weak_ptr<GameObject> owner);

        /// <summary>
        /// Called by GameObject right before this component is detached.
        /// </summary>
        virtual void OnDetach() {}

        /// <summary>
        /// Returns a shared_ptr to the owner, or null if it no longer exists.
        /// </summary>
        std::shared_ptr<GameObject> GetOwner() const;

    private:
        std::weak_ptr<GameObject> m_owner;
    };

} // namespace core
