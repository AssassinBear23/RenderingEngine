#pragma once
#include <memory>

namespace core {

    class GameObject;

    /// <summary>
    /// Base class for components attached to a GameObject.
    /// </summary>
    /// <remarks>
    /// Must keep:
    /// - Components are owned by their GameObject.
    /// - Do not hold a strong pointer back to the owner (use the weak back-reference).
    /// - Components are created as std::shared_ptr (required for RequestDetach()).
    /// </remarks>
    class IComponent : public std::enable_shared_from_this<IComponent> {
    public:
		IComponent() = default;
        virtual ~IComponent() = default;

        /// <summary>
        /// Called when the component is attached to a GameObject.
        /// Stores a weak reference to the owner.
        /// </summary>
        virtual void OnAttach(std::weak_ptr<GameObject> owner) { m_owner = std::move(owner); }

        /// <summary>
		/// Called once in the first frame before Update() is called.
        /// Use to cache references and perform initialization.
        /// </summary>
        virtual void Start() {}

        /// <summary>
        /// Called when the component is enabled.
        /// </summary>
        virtual void OnEnable() {}

        /// <summary>
        /// Called every frame while the component is enabled.
        /// </summary>
        virtual void Update() = 0;

        /// <summary>
        /// Called before the component is removed or the owner is destroyed.
        /// </summary>
        virtual void OnDestroy() {}

        /// <summary>
        /// Enable or disable the component.
        /// </summary>
        void SetEnabled(bool enabled) { m_enabled = enabled; }

        /// <summary>
        /// Returns true if the component is enabled.
        /// </summary>
        bool IsEnabled() const { return m_enabled; }

        /// <summary>
        /// Ask the owning GameObject to detach this exact component instance.
        /// Safe to call from UI (e.g., ImGui) or during Update().
        /// </summary>
        void RequestDetach();

    protected:
        /// <summary>
        /// Returns a shared_ptr to the owner, or null if it no longer exists.
        /// </summary>
        std::shared_ptr<GameObject> GetOwner() const { return m_owner.lock(); }

    private:
        bool m_enabled = true;
        std::weak_ptr<GameObject> m_owner;
    };

} // namespace core
