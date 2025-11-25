#include "Object.h"

namespace core
{
	Object::Object()
	{
		// Set up the callback for isEnabled changes
		isEnabled.SetOnChange([this](bool newValue) {
			OnEnabledChanged(newValue);
		});
	}

	void Object::SetName(std::string n) { m_name = std::move(n); }
	const std::string& Object::GetName() const { return m_name; }

	void Object::OnEnabledChanged(bool newValue)
	{
		if (m_destroyed) return;
		
		if (newValue)
			OnEnable();
		else
			OnDisable();
	}

	void Object::Enable()
	{
		isEnabled = true;
	}

	void Object::Disable()
	{
		isEnabled = false;
	}

	void Object::Destroy()
	{
		if (m_destroyed) return;
		m_destroyed = true; OnDestroy();
	}

	bool Object::IsDestroyed() const { return m_destroyed; }

	void Object::Serialize(nlohmann::json& out) const
	{
		out["name"] = m_name;
		out["enabled"] = isEnabled.Get();
	}

	void Object::Deserialize(const nlohmann::json& in)
	{
		if (in.contains("name")) 
			SetName(in["name"].get<std::string>());
		if (in.contains("enabled"))
			isEnabled = in["enabled"].get<bool>();
	}
} // namespace core