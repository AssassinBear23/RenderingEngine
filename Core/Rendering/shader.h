#pragma once

#include <fstream>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <ios>
#include <iostream>
#include <sstream>
#include <string>
#include <regex>

namespace core
{
    /// <summary>
    /// Shader class for loading, compiling, and managing OpenGL shader programs.
    /// Supports vertex, fragment, and optional geometry shaders.
    /// </summary>
    class Shader
    {
    public:
        /// <summary>
        /// The OpenGL shader program ID.
        /// </summary>
        unsigned int ID;

        Shader() = default;

        /// <summary>
        /// Constructs a shader program from vertex and fragment shader files.
        /// </summary>
        /// <param name="vertexPath">Path to the vertex shader source file.</param>
        /// <param name="fragmentPath">Path to the fragment shader source file.</param>
        Shader(const char* vertexPath, const char* fragmentPath)
        {
            // 1. retrieve the vertex/fragment source code from filePath
            std::string vertexCode;
            std::string fragmentCode;
            std::ifstream vShaderFile;
            std::ifstream fShaderFile;
            // ensure ifstream objects can throw exceptions:
            vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

            try
            {
                // open files
                vShaderFile.open(vertexPath);
                fShaderFile.open(fragmentPath);
                std::stringstream vShaderStream, fShaderStream;
                // read file's buffer contents into streams
                vShaderStream << vShaderFile.rdbuf();
                fShaderStream << fShaderFile.rdbuf();
                // close file handlers
                vShaderFile.close();
                fShaderFile.close();
                // convert stream into string
                vertexCode = ProcessShaderIncludes(vShaderStream.str());
                fragmentCode = ProcessShaderIncludes(fShaderStream.str());
            }
            catch (std::ifstream::failure& e)
            {
                std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
            }
            const char* vShaderCode = vertexCode.c_str();
            const char* fShaderCode = fragmentCode.c_str();
            // 2. compile shaders
            unsigned int vertex, fragment;
            // vertex shader
            vertex = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(vertex, 1, &vShaderCode, NULL);
            glCompileShader(vertex);
            checkCompileErrors(vertex, "VERTEX");
            // fragment Shader
            fragment = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(fragment, 1, &fShaderCode, NULL);
            glCompileShader(fragment);
            checkCompileErrors(fragment, "FRAGMENT");
            // shader Program
            ID = glCreateProgram();
            glAttachShader(ID, vertex);
            glAttachShader(ID, fragment);
            glLinkProgram(ID);
            checkCompileErrors(ID, "PROGRAM");
            // delete the shaders as they're linked into our program now and no longer necessary
            glDeleteShader(vertex);
            glDeleteShader(fragment);

        }

        /// <summary>
        /// Activates the shader program for use in rendering.
        /// </summary>
        void use()
        {
            glUseProgram(ID);
        }

        /// <summary>
        /// Sets a boolean uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="value">The boolean value to set.</param>
        void setBool(const std::string& name, bool value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
        }

        /// <summary>
        /// Sets an integer uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="value">The integer value to set.</param>
        void setInt(const std::string& name, int value) const
        {
            glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
        }

        /// <summary>
        /// Sets a float uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="value">The float value to set.</param>
        void setFloat(const std::string& name, float value) const
        {
            glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
        }

        /// <summary>
        /// Sets a vec2 uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="value">The vec2 value to set.</param>
        void setVec2(const std::string& name, const glm::vec2& value) const
        {
            glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        /// <summary>
        /// Sets a vec2 uniform value in the shader using individual components.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="x">The x component.</param>
        /// <param name="y">The y component.</param>
        void setVec2(const std::string& name, float x, float y) const
        {
            glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
        }

        /// <summary>
        /// Sets a vec3 uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="value">The vec3 value to set.</param>
        void setVec3(const std::string& name, const glm::vec3& value) const
        {
            glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        /// <summary>
        /// Sets a vec3 uniform value in the shader using individual components.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="x">The x component.</param>
        /// <param name="y">The y component.</param>
        /// <param name="z">The z component.</param>
        void setVec3(const std::string& name, float x, float y, float z) const
        {
            glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
        }

        /// <summary>
        /// Sets a vec4 uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="value">The vec4 value to set.</param>
        void setVec4(const std::string& name, const glm::vec4& value) const
        {
            glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
        }

        /// <summary>
        /// Sets a vec4 uniform value in the shader using individual components.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="x">The x component.</param>
        /// <param name="y">The y component.</param>
        /// <param name="z">The z component.</param>
        /// <param name="w">The w component.</param>
        void setVec4(const std::string& name, float x, float y, float z, float w)
        {
            glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
        }

        /// <summary>
        /// Sets a mat2 uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="mat">The 2x2 matrix to set.</param>
        void setMat2(const std::string& name, const glm::mat2& mat) const
        {
            glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        /// <summary>
        /// Sets a mat3 uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="mat">The 3x3 matrix to set.</param>
        void setMat3(const std::string& name, const glm::mat3& mat) const
        {
            glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        /// <summary>
        /// Sets a mat4 uniform value in the shader.
        /// </summary>
        /// <param name="name">The name of the uniform variable.</param>
        /// <param name="mat">The 4x4 matrix to set.</param>
        void setMat4(const std::string& name, const glm::mat4& mat) const
        {
            glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

    private:
        /// <summary>
        /// Checks for shader compilation or program linking errors.
        /// Prints error messages to the console if any errors are found.
        /// </summary>
        /// <param name="shader">The shader or program ID to check.</param>
        /// <param name="type">The type of shader ("VERTEX", "FRAGMENT", "GEOMETRY", or "PROGRAM").</param>
        void checkCompileErrors(GLuint shader, std::string type)
        {
            GLint success;
            GLchar infoLog[1024];
            if (type != "PROGRAM")
            {
                glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
                if (!success)
                {
                    glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
            else
            {
                glGetProgramiv(shader, GL_LINK_STATUS, &success);
                if (!success)
                {
                    glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                    std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                }
            }
        }

        std::string ProcessShaderIncludes(const std::string& source, const std::string& basePath = "assets/shaders/shaderlibrary/")
        {
            std::string result = source;
            std::regex includePattern(R"(#include\s+\"([^\"]+)\")");
            std::smatch match;

            // Keep processing until no more includes found (supports nested includes)
            while (std::regex_search(result, match, includePattern))
            {
                std::string includeFile = match[1].str();
                std::string includePath = basePath + includeFile;

                printf("Processing #include \"%s\" from %s\n", includeFile.c_str(), includePath.c_str());

                std::string includeContent = ReadFileToString(includePath);

                if (includeContent.empty())
                {
                    printf("Warning: Could not read include file: %s\n", includePath.c_str());
                }
                else
                {
                    printf("Successfully loaded include: %s (%zu bytes)\n", includeFile.c_str(), includeContent.size());
                }

                // Replace the #include directive with the file content
                result = std::regex_replace(result, includePattern, includeContent, std::regex_constants::format_first_only);
            }

            return result;
        }

        std::string ReadFileToString(const std::string& filePath)
        {
            std::ifstream fileStream(filePath, std::ios::in);
            if (!fileStream.is_open())
            {
                printf("Could not open file: %s\n", filePath.c_str());
                return "";
            }
            std::stringstream buffer;
            buffer << fileStream.rdbuf();
            return ProcessShaderIncludes(buffer.str());
        }
    };
} // namespace core