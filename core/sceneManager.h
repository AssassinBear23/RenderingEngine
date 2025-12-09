#pragma once

#include "scene.h"
#include <functional>

namespace core
{
    /// <summary>
    /// Manages scene registration and lifecycle.
    /// </summary>
    class SceneManager
    {
    public:
        /// <summary>
        /// Factory function that creates a scene instance.
        /// </summary>
        using SceneFactory = std::function<void(std::shared_ptr<Scene>)>;

        /// <summary>
        /// Creates an empty scene manager.
        /// </summary>
        SceneManager() = default;

        /// <summary>
        /// Destroys the scene manager and cleans up resources.
        /// </summary>
        ~SceneManager() = default;

        /// <summary>
        /// Loads the scene with the specified name.
        /// </summary>
        /// <param name="sceneName">Name of the scene to load.</param>
        /// <param name="uboLights">OpenGL uniform buffer object for lights.</param>
        /// <returns>True if the scene was loaded successfully, false otherwise.</returns>
        bool LoadScene(const std::string& sceneName, GLuint uboLights);

        /// <summary>
        /// 
        /// </summary>
        /// <param name="sceneName"></param>
        /// <returns></returns>
        bool LoadScene(const std::string& sceneName)
        {
            return LoadScene(sceneName, m_internalUbo);
        }

        /// <summary>
        /// Registers a scene factory with the given name.
        /// </summary>
        /// <param name="sceneName">Unique name for the scene.</param>
        /// <param name="factory">Factory function that creates the scene.</param>
        void RegisterScene(const std::string& sceneName, SceneFactory factory);

        /// <summary>
        /// Gets the currently active scene.
        /// </summary>
        /// <returns>Shared pointer to the current scene, or nullptr if none is loaded.</returns>
        std::shared_ptr<Scene> GetCurrentScene() const { return m_currentScene; }

        /// <summary>
        /// Gets all registered scene names.
        /// </summary>
        /// <returns>Vector of scene names.</returns>
        std::vector<std::string> GetSceneNames() const;

    private:
        /// <summary>
        /// The currently active scene.
        /// </summary>
        std::shared_ptr<core::Scene> m_currentScene;

        GLuint m_internalUbo = 0;

        /// <summary>
        /// Map of scene names to their factory functions.
        /// </summary>
        std::unordered_map<std::string, SceneFactory> m_sceneFactories;
    };
}