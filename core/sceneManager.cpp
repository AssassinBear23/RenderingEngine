#include "sceneManager.h"
#include <editor/editor.h>

namespace core
{
    bool SceneManager::LoadScene(const std::string& sceneName, GLuint uboLights)
    {
        if (sceneName.empty()) return false;

        auto it = m_sceneFactories.find(sceneName);
        if (it == m_sceneFactories.end())
            return false;
        
        m_currentScene = std::make_shared<core::Scene>(sceneName);
        editor::Editor::editorCtx.currentScene = m_currentScene;
        editor::Editor::editorCtx.currentSelectedGameObject = nullptr;
        
        if(m_internalUbo != uboLights)
            m_internalUbo = uboLights;

        m_currentScene->SetLightUBO(uboLights);

        // Population of the scene after bare scene creation.
        it->second(m_currentScene);
        return true;
    }

    void SceneManager::RegisterScene(const std::string& sceneName, SceneFactory factory)
    {
        m_sceneFactories[sceneName] = factory;
    }

    std::vector<std::string> SceneManager::GetSceneNames() const
    {
        std::vector<std::string> sceneNames;
        sceneNames.reserve(m_sceneFactories.size());
        for (auto& kvp : m_sceneFactories)
            sceneNames.push_back(kvp.first);

        return sceneNames;
    }
}