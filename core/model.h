#pragma once

#include <vector>
#include <glm/ext/matrix_float4x4.hpp>
#include "Rendering/mesh.h"

namespace core {
    class Model {
    private:
        std::vector<core::Mesh> meshes;
        glm::mat4 modelMatrix;
    public:
        Model(std::vector<core::Mesh> meshes) : meshes(meshes), modelMatrix(1) {}

        void Render(GLenum drawMode);

        void Translate(glm::vec3 translation);
        void Rotate(glm::vec3 axis, float radians);
        void Scale(glm::vec3 Scale);
        glm::mat4 GetModelMatrix() const;
    };
}