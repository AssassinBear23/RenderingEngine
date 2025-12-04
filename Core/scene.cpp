#include "ObjectSystems/Components/Light.h"
#include "ObjectSystems/Components/Renderer.h"
#include "ObjectSystems/GameObject.h"
#include "Scene.h"
#include <algorithm>
#include <glad/glad.h>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/ext/vector_int4.hpp>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace core
{
    Scene::Scene(std::string name)
    {
        SetName(std::move(name));
        depthShader = Shader("shaders/depth_shader.vert", "shaders/depth_shader.frag");
    }

    void Scene::SetName(std::string name) { m_name = std::move(name); }
    const std::string& Scene::GetName() const { return m_name; }

    void Scene::AddRootGameObject(const std::shared_ptr<GameObject>& go)
    {
        if (!go) return;
        if (std::find(m_roots.begin(), m_roots.end(), go) == m_roots.end())
            m_roots.push_back(go);
    }

    void Scene::RemoveRootGameObject(const std::shared_ptr<GameObject>& go)
    {
        if (!go) return;
        m_roots.erase(std::remove(m_roots.begin(), m_roots.end(), go), m_roots.end());
    }

    std::shared_ptr<GameObject> Scene::CreateObject(const std::string& name, const std::shared_ptr<core::GameObject> parent)
    {
        auto newObject = GameObject::Create(name);
        if (parent)
        {
            newObject->SetParent(parent);
        }
        else
        {
            AddRootGameObject(newObject);
        }
        return newObject;
    }

    const std::vector<std::shared_ptr<GameObject>>& Scene::Roots() const { return m_roots; }

    void Scene::Render(const glm::mat4& view, const glm::mat4& projection)
    {
        // Setting light data UBO
        LightData lightData = {};

        for (size_t i = 0; i < m_lights.size() && i < 4; ++i)
        {
            auto light = m_lights[i];
            if (!light || !light->isEnabled) continue;

            auto lightGO = light->GetOwner();
            if (!lightGO || !lightGO->transform) continue;

            lightData.positions[i] = glm::vec4(lightGO->transform->position, 1.0f);
            lightData.directions[i] = glm::vec4(lightGO->transform->forward(), 0.0f);
            lightData.colors[i] = light->GetColor();
            lightData.lightTypes[i] = glm::ivec4(ToInt(light->lightType.Get()), 0, 0, 0);

            CreateShadowMaps(i);
            lightData.numLights++;
        }

        glBindBuffer(GL_UNIFORM_BUFFER, m_uboLights);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightData), &lightData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        RenderFinalScene(view, projection);
    }

    void Scene::CreateShadowMaps(int lightNumber, int resolution)
    {
        glm::mat4 lightProjection, lightView, lightSpaceMatrix;

        float near_plane = 1.0f, far_plane = 25.0f;

        lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        lightView = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        lightSpaceMatrix = lightProjection * lightView;

        // render scene from light's point of view
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, resolution, resolution);
        glBindBuffer(GL_FRAMEBUFFER, depthMapFBOs[lightNumber]);
    }

    void Scene::RenderFinalScene(const glm::mat4& view, const glm::mat4& projection)
    {
        // Rendering all renderers.
        for (const auto& renderer : m_renderers)
        {
            if (!renderer) continue;
            auto go = renderer->GetOwner();
            if (!go || !go->isEnabled || !renderer->isEnabled) continue;

            glm::mat4 worldMatrix = CalculateWorldMatrix(go);
            glm::mat4 mvp = projection * view * worldMatrix;

            if (renderer->GetMaterial())
            {
                renderer->GetMaterial()->SetMat4("mvpMatrix", mvp);
                renderer->GetMaterial()->SetMat4("modelMatrix", worldMatrix);
                renderer->Render();
            }
        }
    }

    glm::mat4 Scene::CalculateWorldMatrix(const std::shared_ptr<GameObject>& go)
    {
        if (!go) return glm::mat4(1.0f);

        // Get local transform
        glm::mat4 localMatrix = glm::mat4(1.0f);
        if (go->transform)
        {
            localMatrix = go->transform->GetLocalMatrix();
        }

        // If has parent, multiply by parent's world matrix
        if (auto parent = go->GetParent().lock())
        {
            return CalculateWorldMatrix(parent) * localMatrix;
        }

        return localMatrix;
    }
}
