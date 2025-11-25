#pragma once
#include <memory>
#include <vector>
#include <string>
#include <algorithm>
#include "Object.h"
#include "Components/Transform.h"

namespace core {

	class Component; // forward
	class Scene;


	/// <summary>
	/// A object present in the scene that can have components and child GameObjects.
	/// </summary>
	/// <remarks>
	/// Must keep:
	/// - Parent holds strong refs to children; child holds a weak ref to parent.
	/// - Components are stored as shared_ptr and detached by reference.
	/// </remarks>
	class GameObject : public Object {
	public:
		std::shared_ptr<core::Transform> transform;

		/// <summary>
		/// Factory Pattern method for creating a new GameObject.
		/// </summary>
		/// <param name="name"></param>
		/// <returns></returns>
		static std::shared_ptr<GameObject> Create(std::string name = {});

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
		/// 
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		std::shared_ptr<T> AddComponent()
		{
			auto c = std::make_shared<T>();
			if (std::dynamic_pointer_cast<Transform>(c) && transform != nullptr)
				return nullptr; // Prevent adding multiple Transform components

			// avoid duplicates of the same instance
			if (std::find(m_components.begin(), m_components.end(), c) == m_components.end()) {
				c->OnAttach(std::static_pointer_cast<GameObject>(shared_from_this()));
				m_components.push_back(c);
			}

			return c;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		bool RemoveComponent()
		{
			auto c = std::dynamic_pointer_cast<Component>(shared_from_this());

			if (std::dynamic_pointer_cast<Transform>(c))
				return false; // Prevent removing Transform component

			for (size_t i = 0; i < m_components.size(); ++i) {
				if (m_components[i].get() == c.get()) {
					m_components[i]->OnDetach();
					m_components.erase(m_components.begin() + i);
					return true;
				}
			}
			return false;
		}

		/// <summary>
		/// 
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <returns></returns>
		template<typename T>
		std::shared_ptr<T> GetComponent() const
		{
			for (const auto& comp : m_components) {
				auto casted = std::dynamic_pointer_cast<T>(comp);
				if (casted) {
					return casted;
				}
			}
			return nullptr;
		}

		/// <summary>
		/// Returns all components attached to this GameObject.
		/// </summary>
		const std::vector<std::shared_ptr<Component>>& GetComponents() const;

		/// <summary>
		/// The scene this GameObject belongs to.
		/// </summary>
		/// <returns>Shared pointer to the scene. Done by locking the weak pointer.</returns>
		const std::shared_ptr<Scene> GetScene() const { return m_scene.lock(); }

		// Json Serialization
		void Serialize(nlohmann::json& out) const override;
		void Deserialize(const nlohmann::json& in) override;

	private:
		/// <summary>
		/// Construct a GameObject. Name optional.
		/// </summary>
		explicit GameObject(std::string name = {});

		/// <summary>
		/// Initialize the GameObject (called by factory method).
		/// </summary>
		void Init()
		{
			transform = AddComponent<Transform>();
		}

        void SetChildrenEnabledState(bool enabled);
        void OnEnabledChanged(bool newValue) override;


		std::weak_ptr<GameObject> m_parent;
		std::vector<std::shared_ptr<GameObject>> m_children;
		std::vector<std::shared_ptr<Component>> m_components;
		std::weak_ptr<Scene> m_scene;
	};


} // namespace core
