#include "GameObject.h"

namespace core
{
	GameObject::GameObject(std::string name)
		: m_name(std::move(name))
	{
		// Generate a unique ID (simple incrementing for now)
		static std::uint32_t nextId = 1;
		m_id = nextId++;

		const auto m_transform = std::static_pointer_cast<core::IComponent>(std::make_shared<core::Transform>());
		attachComponent(m_transform);
	}

	void GameObject::SetParent(std::shared_ptr<GameObject> newParent)
	{
	}

	void GameObject::Update()
	{
	}

	void GameObject::UpdateRecursive()
	{
	}

	std::shared_ptr<GameObject> GameObject::CreateChild(std::string childName)
	{
		return std::shared_ptr<GameObject>();
	}

	void GameObject::attachComponent(const std::shared_ptr<IComponent>& c)
	{
	}
}