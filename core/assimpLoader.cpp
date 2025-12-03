#include "assimpLoader.h"
#include "Rendering/mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace core {
    Model AssimpLoader::loadModel(const std::string& path) {
        printf("Attempting to load model: %s\n", path.c_str());
        
        Assimp::Importer import;
        unsigned int flags = 
            aiProcess_Triangulate |            // Convert all polygons to triangles
            aiProcess_FlipUVs |                // OpenGL UV coordinate system
            aiProcess_CalcTangentSpace |       // For normal mapping (future)
            aiProcess_ValidateDataStructure |  // Validate the imported scene
            aiProcess_ImproveCacheLocality |   // Reorder triangles for GPU cache
            aiProcess_RemoveRedundantMaterials |
            aiProcess_FixInfacingNormals |     // Fix normals pointing inward
            aiProcess_SortByPType |            // Split meshes by primitive type
            aiProcess_FindDegenerates |        // Remove degenerate triangles
            aiProcess_FindInvalidData |        // Remove invalid data
            aiProcess_GenUVCoords;             // Generate UVs if missing
        
        const aiScene *scene = import.ReadFile(path, flags);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            printf("Error loading model [%s]: %s\n", path.c_str(), import.GetErrorString());
            return Model({});
        }

        printf("Model loaded successfully: %s\n", path.c_str());
        printf("  - Number of meshes: %d\n", scene->mNumMeshes);
        printf("  - Number of materials: %d\n", scene->mNumMaterials);
        printf("  - Has animations: %s\n", scene->HasAnimations() ? "YES" : "NO");
        
        std::string directory = path.substr(0, path.find_last_of('/'));
        std::vector<Mesh> meshes;
        processNode(scene->mRootNode, scene, meshes);
        
        printf("  - Processed meshes: %zu\n", meshes.size());
        
        return Model(meshes);
    }

    void AssimpLoader::processNode(aiNode *node, const aiScene *scene, std::vector<Mesh>& meshes) {
        printf("Processing node: %s (meshes: %d, children: %d)\n", 
               node->mName.C_Str(), node->mNumMeshes, node->mNumChildren);
        
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
        
        printf("\n=== PROCESSING MESH ===\n");
        printf("Mesh name: %s\n", mesh->mName.C_Str());
        printf("Vertices: %d\n", mesh->mNumVertices);
        printf("Faces: %d\n", mesh->mNumFaces);
        printf("Primitive types: %d\n", mesh->mPrimitiveTypes);
        printf("Has UVs: %s\n", mesh->HasTextureCoords(0) ? "YES" : "NO");
        printf("Has Normals: %s\n", mesh->HasNormals() ? "YES" : "NO");
        printf("Has Tangents: %s\n", mesh->HasTangentsAndBitangents() ? "YES" : "NO");
        
        // Reserve memory for efficiency
        vertices.reserve(mesh->mNumVertices);
        indices.reserve(mesh->mNumFaces * 3);
        
        // Process vertices
        for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
            glm::vec3 position(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
            
            // Handle normals
            glm::vec3 normal(0.0f, 1.0f, 0.0f); // Default up
            if (mesh->HasNormals()) {
                normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
            }
            
            // Handle UVs
            glm::vec2 uvs(0.0f, 0.0f);
            if (mesh->HasTextureCoords(0)) {
                uvs.x = mesh->mTextureCoords[0][i].x;
                uvs.y = mesh->mTextureCoords[0][i].y;
            }

            glm::vec3 tangent(0.0f);
            if (mesh->HasTangentsAndBitangents()) {
                tangent = glm::vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
            }

            glm::vec3 bitangent(0.0f);
            if (mesh->HasTangentsAndBitangents()) {
                bitangent = glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
            }

            vertices.emplace_back(position, normal, uvs, tangent, bitangent);
        }
        
        // Process indices
        unsigned int skippedFaces = 0;
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            const aiFace& face = mesh->mFaces[i];
            
            if (face.mNumIndices != 3) {
                printf("WARNING: Non-triangle face with %d indices (face %d)!\n", 
                       face.mNumIndices, i);
                skippedFaces++;
                continue;
            }
            
            // Validate indices
            for (unsigned int j = 0; j < 3; j++) {
                unsigned int index = face.mIndices[j];
                if (index >= mesh->mNumVertices) {
                    printf("ERROR: Invalid index %d (max: %d) in face %d!\n", 
                           index, mesh->mNumVertices - 1, i);
                    skippedFaces++;
                    continue;
                }
                indices.push_back(index);
            }
        }
        
        printf("Final vertex count: %zu\n", vertices.size());
        printf("Final index count: %zu\n", indices.size());
        printf("Expected triangles: %zu\n", indices.size() / 3);
        if (skippedFaces > 0) {
            printf("Skipped %d invalid faces\n", skippedFaces);
        }
        printf("=======================\n\n");
        
        return Mesh(vertices, indices);
    }
}