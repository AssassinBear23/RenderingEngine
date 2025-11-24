#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ObjectSystems/Components/Transform.h"
#include <glm/glm.hpp>

namespace core // Forward declaration
{
    class Object;
    class GameObject;
    class Renderer;
}

namespace core {

    /// <summary>
    /// A collection of root GameObjects.
    /// </summary>
    /// <remarks>
    /// Must keep:
    /// - Scene holds strong refs to root GameObjects so they stay alive.
    /// </remarks>
    class Scene
    {
    public:
        /// <summary>
        /// Construct a scene. Name optional.
        /// </summary>
        explicit Scene(std::string name = {});

        /// <summary>
        /// Set the scene name
        /// </summary>
        /// <param name="name">The name to set the scene to.</param>
        void SetName(std::string name);

        /// <summary>
        /// Get the scene name.
        /// </summary>
        /// <returns>The name of the scene.</returns>
        const std::string& GetName() const;

        /// <summary>
        /// Add an existing GameObject as a root.
        /// </summary>
        void AddRootGameObject(const std::shared_ptr<GameObject>& go);

        /// <summary>
        /// 
        /// </summary>
        /// <param name="go"></param>
        void RemoveRootGameObject(const std::shared_ptr<GameObject>& go);

        /// <summary>
        /// Create a new GameObject and set it's parent. If parent is nullptr, it's a root.
        /// </summary>
        /// <param name="name">The name of the Object to create</param>
        /// <param name="parent">The parent GameObject</param>
        std::shared_ptr<GameObject> CreateObject(const std::string& name = "NewObject", const std::shared_ptr<core::GameObject> parent = nullptr);

        /// <summary>
        /// Render all GameObjects in the scene using the given view and projection matrices.
        /// </summary>
        /// <param name="view">The view matrix to pass to the TheRenderGameObject method</param>
        /// <param name="projection">The projection matrix to pass to the TheRenderGameObject method</param>
        void Render(const glm::mat4& view, const glm::mat4& projection);
        
        /// <summary>
        /// Return all root GameObjects.
        /// </summary>
        const std::vector<std::shared_ptr<GameObject>>& Roots() const;

        void RegisterRenderer(const std::shared_ptr<Renderer>& renderer);
        void UnregisterRenderer(const std::shared_ptr<Renderer>& renderer);

    private:
        /// <summary>
        /// Calculate the world matrix for a GameObject.
        /// </summary>
        /// <param name="go"></param>
        /// <returns></returns>
        glm::mat4 CalculateWorldMatrix(const std::shared_ptr<GameObject>& go);

        std::string m_name;
        std::vector<std::shared_ptr<GameObject>> m_roots;
        std::vector<std::shared_ptr<Renderer>> m_renderers;
    };

} // namespace core
