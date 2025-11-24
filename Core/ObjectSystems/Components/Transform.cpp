#include "transform.h"
#include "../ComponentFactory.h"

namespace core
{
	// Registers the Transform component with the component factory
	REGISTER_COMPONENT(Transform);

	glm::mat4 core::Transform::GetLocalMatrix() const
	{
		glm::mat4 mat(1.0f);
		mat = glm::translate(mat, position);
		mat = glm::rotate(mat, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		mat = glm::rotate(mat, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		mat = glm::rotate(mat, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		mat = glm::scale(mat, scale);
		return mat;
	}

	void Transform::Serialize(nlohmann::json& out) const {
		Component::Serialize(out);
		out["position"] = { position.x, position.y, position.z };
		out["rotation"] = { rotation.x, rotation.y, rotation.z };
		out["scale"] = { scale.x, scale.y, scale.z };
	}

	void Transform::Deserialize(const nlohmann::json& in) {
		Component::Deserialize(in);
		if (in.contains("position") && in["position"].is_array())
			position = glm::vec3(in["position"][0], in["position"][1], in["position"][2]);
		if (in.contains("rotation") && in["rotation"].is_array())
			rotation = glm::vec3(in["rotation"][0], in["rotation"][1], in["rotation"][2]);
		if (in.contains("scale") && in["scale"].is_array())
			scale = glm::vec3(in["scale"][0], in["scale"][1], in["scale"][2]);
	}
} // namespace core