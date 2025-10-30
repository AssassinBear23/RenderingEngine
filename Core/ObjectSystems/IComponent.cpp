#include "IComponent.h"
#include "GameObject.h"

namespace core
{
	void core::IComponent::RequestDetach()
	{
		if (auto owner = GetOwner())
		{
			owner.get()->RemoveComponent(shared_from_this());
		}
	}
}