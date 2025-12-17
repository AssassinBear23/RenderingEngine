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

        /// <summary>
        /// Associates a Texture object with a shader uniform and texture unit.
        /// When Use() is called, the texture will be automatically bound to the specified slot
        /// and the uniform will be set to that slot number.
        /// Use this when working with Texture objects.
        /// </summary>
        /// <param name="uniformName">The name of the sampler uniform in the shader (e.g., "diffuseMap")</param>
        /// <param name="texture">Shared pointer to the Texture object to bind</param>
        /// <param name="slot">The texture unit slot (0-31) to bind the texture to</param>
        void SetTexture(const std::string& uniformName, const std::shared_ptr<Texture>& texture, int slot) {
            m_textures[uniformName] = { texture, slot };
        }
        
        std::shared_ptr<Texture> GetTexture(const std::string& uniformName) const
        {
            auto it = m_textures.find(uniformName);
            return it != m_textures.end() ? it->second.texture : nullptr;
        }
        
        /// <summary>
        /// Sets a float uniform value.
        /// </summary>
        /// <param name="name">The name of the uniform in the shader</param>
        /// <param name="value">The float value to set</param>
        void SetFloat(const std::string& name, float value) { m_floats[name] = value; }
        
        /// <summary>
        /// Sets an integer uniform value.
        /// Common uses: texture unit indices (when manually binding textures), 
        /// flags, counts, or other integer shader parameters.
        /// For texture samplers: Use this when you've manually called glBindTexture() 
        /// and need to tell the shader which texture unit to sample from.
        /// </summary>
        /// <param name="name">The name of the uniform in the shader</param>
        /// <param name="value">The integer value to set (e.g., texture unit number 0-31)</param>
        void SetInt(const std::string& name, int value) { m_ints[name] = value; }
        
        /// <summary>
        /// Sets a boolean uniform value (internally converted to int: 0 or 1).
        /// </summary>
        /// <param name="name">The name of the uniform in the shader</param>
        /// <param name="value">The boolean value to set</param>
        void SetBool(const std::string& name, bool value) { m_bools[name] = value ? 1 : 0; }
        
        /// <summary>
        /// Sets a vec3 uniform value.
        /// Common uses: colors, positions, directions, normals.
        /// </summary>
        /// <param name="name">The name of the uniform in the shader</param>
        /// <param name="value">The vec3 value to set</param>
        void SetVec3(const std::string& name, const glm::vec3& value) { m_vec3s[name] = value; }
        
        /// <summary>
        /// Sets a vec4 uniform value.
        /// Common uses: colors with alpha, positions with w component.
        /// </summary>
        /// <param name="name">The name of the uniform in the shader</param>
        /// <param name="value">The vec4 value to set</param>
        void SetVec4(const std::string& name, const glm::vec4& value) { m_vec4s[name] = value; }
        
        /// <summary>
        /// Sets a mat4 uniform value.
        /// Common uses: transformation matrices (model, view, projection).
        /// </summary>
        /// <param name="name">The name of the uniform in the shader</param>
        /// <param name="value">The mat4 value to set</param>
        void SetMat4(const std::string& name, const glm::mat4& value) { m_mat4s[name] = value; }

        /// <summary>
        /// Bind the shader program and set all uniforms and textures.
        /// Call this before rendering with this material.
        /// </summary>
        void Use() const;

    private:
        GLuint m_shaderProgram = 0;
        struct TextureData
        {
            std::shared_ptr<Texture> texture;
            int slot = 0;
        };

        std::unordered_map<std::string, TextureData> m_textures;
        std::unordered_map<std::string, float> m_floats;
        std::unordered_map<std::string, int> m_ints;
        std::unordered_map<std::string, bool> m_bools;
        std::unordered_map<std::string, glm::vec3> m_vec3s;
        std::unordered_map<std::string, glm::vec4> m_vec4s;
        std::unordered_map<std::string, glm::mat4> m_mat4s;
    };
} // namespace core