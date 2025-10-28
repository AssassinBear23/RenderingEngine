#pragma once
namespace core
{
	class IComponent
	{
	public:
		virtual ~IComponent() = default;
		virtual void Update() = 0;
		//TODO: implement
	};
}
