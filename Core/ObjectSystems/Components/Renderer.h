#pragma once
#include "../Component.h"
#include "../../Rendering/mesh.h"
#include "../../material.h"
#include <memory>
#include <vector>

namespace core
{
    class Scene; // Forward declaration

    /// <summary>
    /// Renderer component that holds meshes and a material for rendering.
    /// Combines mesh data and rendering properties in one component.
    /// </summary>
    class Renderer : public Component
    {
    public:
        Renderer() = default;
        
		std::string GetTypeName() const override { return "Renderer"; }

        // Single mesh constructor
        Renderer(const Mesh& mesh, std::shared_ptr<Material> material)
            : m_meshes{mesh}, m_material(material) {}

        // Multiple meshes constructor (for models with submeshes)
        Renderer(const std::vector<Mesh>& meshes, std::shared_ptr<Material> material)
            : m_meshes(meshes), m_material(material) {}

        // Mesh management
        void SetMesh(const Mesh& mesh) { 
            m_meshes.clear();
            m_meshes.push_back(mesh);
        }

        void SetMeshes(const std::vector<Mesh>& meshes) { 
            m_meshes = meshes;
        }

        const std::vector<Mesh>& GetMeshes() const { return m_meshes; }

        // Material management
        void SetMaterial(std::shared_ptr<Material> material) { m_material = material; }
        std::shared_ptr<Material> GetMaterial() const { return m_material; }

        /// <summary>
        /// Render all meshes with the current material.
        /// </summary>
        /// <param name="drawMode">OpenGL draw mode (GL_TRIANGLES, etc.)</param>
        void Render(GLenum drawMode = GL_TRIANGLES);

        void DrawGui() override;

        ///
        void OnAttach(std::weak_ptr<GameObject> owner) override;

        /// <summary>
        /// 
        /// </summary>
        void OnDetach() override;

    private:
        std::vector<Mesh> m_meshes;
        std::shared_ptr<Material> m_material;
        std::weak_ptr<Scene> m_scene;
    };
}
