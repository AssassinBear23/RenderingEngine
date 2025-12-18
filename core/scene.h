#pragma once
#include <glad/glad.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <memory>
#include <string>
#include <vector>
#include "Rendering/shader.h"

namespace core // Forward declaration
{
    class Object;
    class GameObject;
    class Renderer;
    class Light;
}

namespace core
{
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
        explicit Scene(std::string name = { });

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
        /// Remove a GameObject from the root GameObjects list.
        /// </summary>
        /// <param name="go">The GameObject to remove from roots.</param>
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

        // Convenience methods for specific component types
        void RegisterRenderer(const std::shared_ptr<Renderer>& renderer) { RegisterComponent(renderer, m_renderers); }
        void UnregisterRenderer(const std::shared_ptr<Renderer>& renderer) { UnregisterComponent(renderer, m_renderers); }

        void RegisterLight(const std::shared_ptr<Light>& light) { RegisterComponent(light, m_lights); }
        void UnregisterLight(const std::shared_ptr<Light>& light) { UnregisterComponent(light, m_lights); }

        void SetLightUBO(GLuint ubo) { m_uboLights = ubo; }

        /// <summary>
        /// Sets the bloom threshold value for all lit materials in the scene.
        /// This controls which pixels contribute to the bloom effect in the MRT output.
        /// </summary>
        /// <param name="threshold">The brightness threshold (default: 1.0)</param>
        void SetBloomThreshold(float threshold) { m_bloomThreshold = threshold; }

        // Accessor methods
        const std::vector<std::shared_ptr<Renderer>>& GetRenderers() const { return m_renderers; }
        const std::vector<std::shared_ptr<Light>>& GetLights() const { return m_lights; }

    private:
        /// <summary>
        /// Generic registration for components
        /// </summary>
        template<typename T>
        void RegisterComponent(const std::shared_ptr<T>& component, std::vector<std::shared_ptr<T>>& container)
        {
            if (!component) return;

            if (std::find(container.begin(), container.end(), component) == container.end())
                container.push_back(component);
        }

        /// <summary>
        /// Generic unregistration for components
        /// </summary>
        template<typename T>
        void UnregisterComponent(const std::shared_ptr<T>& component, std::vector<std::shared_ptr<T>>& container)
        {
            if (!component) return;
            container.erase(std::remove(container.begin(), container.end(), component), container.end());
        }

        void RenderShadowMap(int lightIndex);
        void RenderFinalScene(const glm::mat4& view, const glm::mat4& projection);
        void GenerateDepthMaps(int numLights, int width_resolution, int height_resolution);

        /// <summary>
        /// Calculate the world matrix for a GameObject.
        /// </summary>
        /// <param name="go"></param>
        /// <returns></returns>
        glm::mat4 CalculateWorldMatrix(const std::shared_ptr<GameObject>& go);

        std::string m_name;
        std::vector<std::shared_ptr<GameObject>> m_roots;
        std::vector<std::shared_ptr<Light>> m_lights;
        GLuint m_uboLights{ 0 };
        std::vector<std::shared_ptr<Renderer>> m_renderers;
        std::vector<glm::mat4> m_lightSpaceMatrices;
        core::Shader depthShader;
        std::vector<unsigned int> m_depthMapFBOs;
        std::vector<unsigned int> m_depthMaps;
        const int SHADOW_WIDTH = 1024;
        const int SHADOW_HEIGHT = 1024;
        float m_bloomThreshold = 1.0f;  // Default bloom threshold
    };

} // namespace core
