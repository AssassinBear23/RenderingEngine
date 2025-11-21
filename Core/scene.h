#pragma once
#include <memory>
#include <string>
#include <vector>
#include "ObjectSystems/Object.h"
#include "ObjectSystems/GameObject.h"

namespace core {

    /// <summary>
    /// A collection of root GameObjects.
    /// </summary>
    /// <remarks>
    /// Must keep:
    /// - Scene holds strong refs to root GameObjects so they stay alive.
    /// </remarks>
    class Scene : public Object {
    public:
        /// <summary>
        /// Construct a scene. Name optional.
        /// </summary>
        explicit Scene(std::string name = {});

        /// <summary>
        /// Add an existing GameObject as a root.
        /// </summary>
        void AddRootGameObject(const std::shared_ptr<GameObject>& go);

        /// <summary>
        /// Create a new GameObject and set it's parent. If parent is nullptr, it's a root.
        /// </summary>
        /// <param name="name">The name of the Object to create</param>
        /// <param name="parent">The parent GameObject</param>
        std::shared_ptr<GameObject> CreateObject(const std::string& name = "NewObject", const std::shared_ptr<core::GameObject> parent = nullptr);

        /// <summary>
        /// Return all root GameObjects.
        /// </summary>
        const std::vector<std::shared_ptr<GameObject>>& Roots() const;

        void Draw();

    private:
        std::vector<std::shared_ptr<GameObject>> m_roots;
    };

} // namespace core
