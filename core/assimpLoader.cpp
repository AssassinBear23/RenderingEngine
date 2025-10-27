#include "assimpLoader.h"
#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace core {
    Model AssimpLoader::loadModel(const std::string& path) {
        Assimp::Importer import;
        const aiScene *scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            printf("Error: %s\n", import.GetErrorString());
            return Model({});
        }

        std::string directory = path.substr(0, path.find_last_of('/'));
        std::vector<Mesh> meshes;
        processNode(scene->mRootNode, scene, meshes);
        return Model(meshes);
    }

    void AssimpLoader::processNode(aiNode *node, const aiScene *scene, std::vector<Mesh>& meshes) {
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene, meshes);
        }
    }

    Mesh AssimpLoader::processMesh(aiMesh *mesh, const aiScene *scene) {
        std::vector<Vertex> vertices;
        std::vector<GLuint> indices;
        glm::vec2 uvs(0, 0);
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            if (mesh->mTextureCoords[0] != nullptr) {
                aiVector3D assimpUVs = mesh->mTextureCoords[0][i];
                uvs.x = assimpUVs.x;
                uvs.y = assimpUVs.y;
            } else {
                uvs.x = 0.0f;
                uvs.y = 0.0f;
            }
            vertices.emplace_back(glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z), glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z), uvs);
        }
        aiFace face;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(face.mIndices[j]);
            }
        }
        return Mesh(vertices, indices);
    }
}