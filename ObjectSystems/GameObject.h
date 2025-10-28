#pragma once

#include <vector>
#include <memory>
#include "IComponent.h"

namespace core
{
	/// <summary>
	/// Represents a Object in the scene, with components and behaviour added as modules.
	/// <para>
	/// A `GameObject` can have a single parent, multiple children, and one or more components implementing `IComponent` to provide behavior.
	/// </para>
	/// </summary>
	/// <remarks>
	/// Ownership/Lifetime:
	/// - `parent` is a `std::shared_ptr` to the parent node. Children hold a strong reference to the parent.
	/// - `children` are stored as `std::weak_ptr` to avoid reference cycles (parent strong -> child weak).
	/// 
	/// Responsibilities:
	/// - Orchestrates update propagation to its attached components.
	/// - Manages parent-child relationships, keeping child references weak to prevent cycles.
	/// 
	/// Thread-safety:
	/// - Not thread-safe. Access/modify only on the main/game thread unless externally synchronized.
	/// </remarks>
	class GameObject
	{
	public:
		/// <summary>
		/// Strong reference to this object's parent. Can be `nullptr` for root objects.
		/// </summary>
		std::shared_ptr<GameObject> parent;
		/// <summary>
		/// Weak references to this object's children. Weak pointers prevent reference cycles.
		/// Expired entries may be present and should be pruned or filtered by callers when locking.
		/// </summary>
		std::vector<std::weak_ptr<GameObject>> children;
		/// <summary>
		/// Collection of components attached to this `GameObject`.
		/// </summary>
		/// <remarks>
		/// Ownership/Lifetime:
		/// - Stored as `std::shared_ptr<IComponent>`; this `GameObject` holds strong references.
		/// - Avoid strong back-references from components to their owner to prevent cycles (prefer `std::weak_ptr`).
		///
		/// Ordering/Determinism:
		/// - Preserves insertion order; implementations typically call `Update()` on components in this order for deterministic behavior.
		///
		/// Uniqueness:
		/// - No inherent constraint on duplicates; multiple components of the same type are allowed unless restricted by higher-level logic.
		/// </remarks>
		std::vector<std::shared_ptr<IComponent>> components;

	private:
		/// <summary>
		/// Opaque, stable identifier for this `GameObject`.
		/// </summary>
		/// <remarks>
		/// Assignment and uniqueness scope are implementation-defined. Typical implementations assign during construction
		/// and keep it immutable and unique within the process, scene, or world.
		/// </remarks>
		std::uint32_t id;

	public:
		/// <summary>
		/// Constructs a `GameObject` with an optional parent and an optional initial component.
		/// </summary>
		/// <param name="parent">
		/// The parent to assign at construction. Pass `nullptr` to create a root object.
		/// If non-null, the implementation is expected to reflect this relationship in the parent's `children`.
		/// </param>
		/// <param name="components">
		/// An optional initial component to attach to this `GameObject`. Pass `nullptr` for none.
		/// The storage/ownership model for components is determined by the implementation (e.g., stored internally).
		/// </param>
		/// <remarks>
		/// This constructor does not define whether it automatically registers the new object with `parent->children`;
		/// implementations should document and ensure consistency with `SetParent`.
		/// </remarks>
		GameObject(std::shared_ptr<GameObject> parent = nullptr, std::vector<std::shared_ptr<IComponent>> components = {});

		/// <summary>
		/// Gets this object's identifier.
		/// </summary>
		/// <returns>
		/// The unique numeric identifier for this `GameObject`. This value is intended to be stable
		/// for the lifetime of the object and should be treated as read-only.
		/// </returns>
		/// <remarks>
		/// The scope of uniqueness (e.g., process-wide, scene-local) is implementation-defined.
		/// Prefer using this ID for identity/equality checks. Avoid using it for persistent serialization
		/// unless a stable mapping strategy is provided by higher-level systems.
		/// </remarks>
		std::uint32_t GetId() const { return id; }
		
		/// <summary>
		/// Updates this game object. Typical implementations invoke `Update()` on all attached components,
		/// and may optionally propagate updates to children.
		/// </summary>
		/// <remarks>
		/// The exact update semantics (order, propagation to children, early-out conditions) are left to the implementation.
		/// Implementations should consider:
		/// - A stable iteration order of components (if required by game logic).
		/// - Skipping or pruning expired child references before propagation.
		/// - Avoiding re-entrance and ensuring thread confinement to the main/game thread.
		/// </remarks>
		void Update();

		/// <summary>
		/// Sets a new parent for this game object, updating both this object's `parent` field and the
		/// previous/new parents' `children` collections accordingly.
		/// </summary>
		/// <param name="newParent">The new parent to assign. Pass `nullptr` to detach and become a root object.</param>
		/// <remarks>
		/// Implementations should:
		/// - Remove this object from the old parent's `children` list (if any).
		/// - Add this object to the new parent's `children` list (if non-null).
		/// - Maintain weak child references to avoid cycles.
		/// - Safely handle cases where `newParent == parent` (no-op).
		/// - Ensure no duplicate entries exist in the parent's `children` collection.
		/// </remarks>
		void SetParent(std::shared_ptr<GameObject> const newParent);

		/// <summary>
		/// Returns strong references to the current (non-expired) children.
		/// </summary>
		/// <returns>
		/// A vector of `std::shared_ptr<GameObject>` for all children that could be successfully locked.
		/// Expired weak entries are omitted.
		/// </returns>
		/// <remarks>
		/// Performance: Locks each weak child once; complexity is O(N) with N = number of stored children.
		/// Callers may wish to cache results if used frequently within a frame.
		/// </remarks>
		std::vector<std::shared_ptr<GameObject>> GetChildren() const;

		/// <summary>
		/// Gets the number of currently valid (non-expired) children.
		/// </summary>
		/// <returns>The count of children whose weak references can be locked.</returns>
		/// <remarks>
		/// Equivalent to `GetChildren().size()` in typical implementations, but may be optimized to avoid allocations.
		/// </remarks>
		int GetChildrenCount() const;
	};
}