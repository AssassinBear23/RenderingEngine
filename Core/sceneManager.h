#pragma once

#include "scene.h"

namespace core
{
    /// <summary>
    /// Manages scene registration, loading, and lifecycle within the application.
    /// Provides a centralized system for switching between different scenes using a factory pattern.
    /// </summary>
    /// <remarks>
    /// The SceneManager uses a factory pattern to defer scene instantiation until needed.
    /// This approach allows for efficient memory management and lazy initialization of game scenes.
    /// </remarks>
    class SceneManager
    {
    public:
        /// <summary>
        /// Factory function type for creating scene instances.
        /// </summary>
        /// <remarks>
        /// The factory function should return a valid shared pointer to a Scene-derived object.
        /// This allows for deferred instantiation and polymorphic scene creation.
        /// </remarks>
        using SceneFactory = std::function<void(std::shared_ptr<Scene>)>;

        /// <summary>
        /// Default constructor for SceneManager.
        /// </summary>
        /// <remarks>
        /// Initializes an empty scene manager with no registered scenes.
        /// Scenes must be registered before they can be loaded.
        /// </remarks>
        SceneManager() = default;

        /// <summary>
        /// Default destructor for SceneManager.
        /// </summary>
        /// <remarks>
        /// Cleans up the current scene and all registered factory functions.
        /// Scene cleanup is handled automatically through shared_ptr reference counting.
        /// </remarks>
        ~SceneManager() = default;

        /// <summary>
        /// Loads and activates the scene with the specified name.
        /// If the scene exists in the registry, it becomes the current active scene.
        /// </summary>
        /// <param name="sceneName">The name identifier of the scene to load.</param>
        /// <returns>
        /// true if the scene was successfully loaded and activated;
        /// false if the scene name was not found in the registry or creation failed.
        /// </returns>
        /// <remarks>
        /// This method performs the following operations:
        /// 1. Searches for the scene in the registered factory map
        /// 2. If found, invokes the factory function to create a new scene instance
        /// 3. Replaces the current scene with the newly created scene
        /// 
        /// The previous scene is automatically cleaned up when replaced.
        /// If the factory returns nullptr, the load operation fails.
        /// </remarks>
        bool LoadScene(const std::string& sceneName);

        /// <summary>
        /// Registers a new scene with the manager using a factory function.
        /// The factory allows deferred instantiation of scenes until they are loaded.
        /// </summary>
        /// <param name="sceneName">The unique name identifier for the scene.</param>
        /// <param name="factory">Factory function that creates and returns a shared pointer to the scene.</param>
        /// <remarks>
        /// Registering a scene does not instantiate it immediately.
        /// The factory function is stored and called later when LoadScene() is invoked.
        /// 
        /// If a scene with the same name already exists, it will be overwritten.
        /// The factory function should be lightweight and stateless.
        /// 
        /// Example:
        /// RegisterScene("MainMenu", []() { return std::make_shared<MainMenuScene>(); });
        /// </remarks>
        void RegisterScene(const std::string& sceneName, SceneFactory factory);

        /// <summary>
        /// Retrieves the currently active scene.
        /// </summary>
        /// <returns>A shared pointer to the current scene, or nullptr if no scene is loaded.</returns>
        /// <remarks>
        /// The returned pointer is shared, allowing multiple systems to safely reference the active scene.
        /// Returns nullptr if no scene has been loaded yet via LoadScene().
        /// </remarks>
        std::shared_ptr<Scene> GetCurrentScene() const { return m_currentScene; }

        /// <summary>
        /// Retrieves a list of all registered scene names.
        /// </summary>
        /// <returns>A vector containing the names of all available scenes in the registry.</returns>
        /// <remarks>
        /// Useful for debugging, UI scene selection, or validating scene availability.
        /// The order of names in the returned vector is not guaranteed.
        /// </remarks>
        std::vector<std::string> GetSceneNames() const;

    private:
        /// <summary>
        /// The currently active scene instance.
        /// </summary>
        /// <remarks>
        /// Holds a shared pointer to the active scene. May be nullptr if no scene is loaded.
        /// Automatically cleaned up when a new scene is loaded or the manager is destroyed.
        /// </remarks>
        std::shared_ptr<core::Scene> m_currentScene;

        /// <summary>
        /// Registry of all available scenes mapped by their unique names.
        /// </summary>
        /// <remarks>
        /// Key: Scene name identifier (string)
        /// Value: Factory function that creates the scene instance
        /// 
        /// Uses unordered_map for O(1) average lookup performance when loading scenes.
        /// </remarks>
        std::unordered_map<std::string, SceneFactory> m_sceneFactories;
    };
}