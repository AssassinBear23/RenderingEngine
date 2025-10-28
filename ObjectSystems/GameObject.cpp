#include "GameObject.h"

namespace core
{
	GameObject::GameObject(std::shared_ptr<GameObject> parent, std::vector<std::shared_ptr<IComponent>> components)
	{
		this->parent = parent;
		this->components = components;
	}

	void GameObject::Update()
	{
		//TODO: implement Update functionality
	}

	void GameObject::SetParent(std::shared_ptr<GameObject> const newParent)
	{
		parent = newParent;
	}

	std::vector<std::shared_ptr<GameObject>> GameObject::GetChildren() const // Const here means that *this object cant be modified.
	{
		std::vector<std::shared_ptr<GameObject>> result;
		result.reserve(children.size());
		for (const auto& child : children)
		{
			if (auto childPtr = child.lock()) // Check that the object still exists.
			{
				result.push_back(childPtr);
			}
		}
		return result;
	}

	int GameObject::GetChildrenCount() const
	{
		return children.size();
	}
}