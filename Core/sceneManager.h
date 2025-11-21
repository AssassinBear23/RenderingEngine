#pragma once

#include "scene.h"

namespace core
{
    class SceneManager
    {
    public:
        Scene CurrentActiveScene;
        SceneManager() = default;
        ~SceneManager() = default;
        void LoadScene(const std::string& sceneName);
        // Add methods for managing scenes, game objects, etc.
    private:
        // Private members for storing scenes, game objects, etc.
    };
}