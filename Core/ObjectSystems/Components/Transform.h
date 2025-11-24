#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "../component.h"

namespace core
{
    class Transform : public Component
    {
    public:
        std::string GetTypeName() const override { return "Transform"; }

        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 rotation = glm::vec3(0.0f); // Euler Angles
        glm::vec3 scale = glm::vec3(1.0f);

        Transform() = default;

        glm::mat4 GetLocalMatrix() const;
        
        // Serialization
        void Serialize(nlohmann::json& out) const override;
        void Deserialize(const nlohmann::json& in) override;
    };
} // namespace core