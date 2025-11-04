#include "Object.h"

namespace core
{
	void Object::SetName(std::string n) { m_name = std::move(n); }
	const std::string& Object::GetName() const { return m_name; }

	void Object::Enable()
	{
		if (m_enabled || m_destroyed) return;
		m_enabled = true;  OnEnable();
	}
	void Object::Disable()
	{
		if (!m_enabled || m_destroyed) return;
		m_enabled = false; OnDisable();
	}

	void Object::Destroy()
	{
		if (m_destroyed) return;
		m_destroyed = true; OnDestroy();
	}

	bool Object::IsEnabled()   const { return m_enabled; }
	bool Object::IsDestroyed() const { return m_destroyed; }
}