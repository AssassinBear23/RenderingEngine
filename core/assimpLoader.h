#pragma once

#include <string>
#include <assimp/scene.h>
#include "mesh.h"
#include "model.h"

namespace core {

    class AssimpLoader {
    public:
        static Model loadModel(const std::string& path);
    private:
        static void processNode(aiNode* node, const aiScene* scene, std::vector<Mesh>& meshes);
        static Mesh processMesh(aiMesh *mesh, const aiScene *scene);
    };

} // core

