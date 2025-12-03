#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include "Rendering/texture.h"

namespace core
{
    class Material {
    public:
        Material() = default;
        explicit Material(GLuint shaderProgram) : m_shaderProgram(shaderProgram) {}
        void SetShaderProgram(GLuint program) { m_shaderProgram = program; }
        GLuint GetShaderProgram() const { return m_shaderProgram; }

        // Texture management
        void SetTexture(const std::string& uniformName, const std::shared_ptr<Texture>& texture, int slot) {
            m_textures[uniformName] = { texture, slot };
        }
        std::shared_ptr<Texture> GetTexture(const std::string& uniformName) const
        {
            auto it = m_textures.find(uniformName);
            return it != m_textures.end() ? it->second.texture : nullptr;
        }
        void SetFloat(const std::string& name, float value) { m_floats[name] = value; }
        void SetBool(const std::string& name, bool value) { m_bools[name] = value ? 1 : 0; }
        void SetVec3(const std::string& name, const glm::vec3& value) { m_vec3s[name] = value; }
        void SetVec4(const std::string& name, const glm::vec4& value) { m_vec4s[name] = value; }
        void SetMat4(const std::string& name, const glm::mat4& value) { m_mat4s[name] = value; }

        /// <summary>
        /// Bind the shader program and set all uniforms and textures.
        /// </summary>
        void Use() const;

    private:
        GLuint m_shaderProgram = 0;
        struct TextureData
        {
            std::shared_ptr<Texture> texture;
            int slot;
        };

        std::unordered_map<std::string, TextureData> m_textures;
        std::unordered_map<std::string, float> m_floats;
        std::unordered_map<std::string, bool> m_bools;
        std::unordered_map<std::string, glm::vec3> m_vec3s;
        std::unordered_map<std::string, glm::vec4> m_vec4s;
        std::unordered_map<std::string, glm::mat4> m_mat4s;
    };
} // namespace core