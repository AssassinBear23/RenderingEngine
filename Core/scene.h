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
        /// Create a root GameObject and return it.
        /// </summary>
        std::shared_ptr<GameObject> CreateRoot(std::string name = {});

        /// <summary>
        /// Add an existing GameObject as a root.
        /// </summary>
        void AddRoot(const std::shared_ptr<GameObject>& go);

        /// <summary>
        /// Return all root GameObjects.
        /// </summary>
        const std::vector<std::shared_ptr<GameObject>>& Roots() const;

    private:
        std::vector<std::shared_ptr<GameObject>> m_roots;
    };

} // namespace core
