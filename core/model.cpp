#include "model.h"
#include <glm/gtc/matrix_transform.hpp>

namespace core {
    void core::Model::render(GLenum drawMode) {
        for (int i = 0; i < meshes.size(); ++i) {
            meshes[i].render(drawMode);
        }
    }

    void Model::translate(glm::vec3 translation) {
        modelMatrix = glm::translate(modelMatrix, translation);
    }

    void Model::rotate(glm::vec3 axis, float radians) {
        modelMatrix = glm::rotate(modelMatrix, radians, axis);
    }

    void Model::scale(glm::vec3 scale) {
        modelMatrix = glm::scale(modelMatrix, scale);
    }

    glm::mat4 Model::getModelMatrix() const {
        return this->modelMatrix;
    }
}