#include "material.h"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.inl>


namespace core
{
    void Material::Use() const
    {
        glUseProgram(m_shaderProgram);

        // Bind Texture objects
        for (const auto& [name, texData] : m_textures)
        {
            if (texData.texture)
            {
                glActiveTexture(GL_TEXTURE0 + texData.slot);
                glBindTexture(GL_TEXTURE_2D, texData.texture->getId());
                GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                if (location != -1)
                {
                    glUniform1i(location, texData.slot);
                }
            }
        }

        // Bind raw texture IDs
        for (const auto& [name, texData] : m_rawTextures)
        {
            if (texData.textureID != 0)
            {
                glActiveTexture(GL_TEXTURE0 + texData.slot);
                glBindTexture(GL_TEXTURE_2D, texData.textureID);
                GLint location = glGetUniformLocation(m_shaderProgram, name.c_str());
                if (location != -1)
                {
                    glUniform1i(location, texData.slot);
                }
            }
        }

        // Set uniforms
        for (const auto& [name, value] : m_floats)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform1f(loc, value);
        }

        for (const auto& [name, value] : m_ints)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform1i(loc, value);
        }

        for (const auto& [name, value] : m_bools)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform1i(loc, value);
        }

        for (const auto& [name, value] : m_vec3s)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform3fv(loc, 1, glm::value_ptr(value));
        }

        for (const auto& [name, value] : m_vec4s)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniform4fv(loc, 1, glm::value_ptr(value));
        }

        for (const auto& [name, value] : m_mat4s)
        {
            GLint loc = glGetUniformLocation(m_shaderProgram, name.c_str());
            if (loc != -1) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
        }
    }
}