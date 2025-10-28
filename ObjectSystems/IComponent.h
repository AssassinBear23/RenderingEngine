#pragma once
namespace core
{
	class IComponent
	{
		virtual ~IComponent() = default;
		virtual void Update() = 0;
		//TODO: implement
	};
}
