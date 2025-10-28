#include "model.h"
#include <glm/gtc/matrix_transform.hpp>

namespace core {
    void core::Model::Render(GLenum drawMode) {
        for (int i = 0; i < meshes.size(); ++i) {
            meshes[i].Render(drawMode);
        }
    }

    void Model::Translate(glm::vec3 translation) {
        modelMatrix = glm::translate(modelMatrix, translation);
    }

    void Model::Rotate(glm::vec3 axis, float radians) {
        modelMatrix = glm::rotate(modelMatrix, radians, axis);
    }

    void Model::Scale(glm::vec3 Scale) {
        modelMatrix = glm::scale(modelMatrix, Scale);
    }

    glm::mat4 Model::GetModelMatrix() const {
        return this->modelMatrix;
    }
}