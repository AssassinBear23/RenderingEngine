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
        depthShader = Shader("assets/shaders/depthVertex.vert", "assets/shaders/depthFragment.frag");
        // printf("[Scene] Created scene: %s\n", m_name.c_str());
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
        // printf("\n=== Scene::Render START ===\n");
        // printf("[Render] Renderers: %zu, Lights: %zu\n", m_renderers.size(), m_lights.size());
        
        if (m_renderers.empty())
        {
            // printf("[WARNING] No renderers registered in scene!\n");
            return;
        }

        // Setting light data UBO
        LightData lightData = {};
        lightData.numLights = static_cast<int>(m_lights.size() < 4 ? m_lights.size() : 4);

        if (m_depthMaps.size() < lightData.numLights)
        {
            // printf("[Render] Generating depth maps for %d lights\n", lightData.numLights);
            GenerateDepthMaps(lightData.numLights, SHADOW_WIDTH, SHADOW_HEIGHT);
        }

        // Save current viewport dimensions AND framebuffer binding
        GLint viewport[4];
        GLint previousFramebuffer;
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFramebuffer);
        // printf("[Render] Current viewport: %d x %d at (%d, %d), Framebuffer: %d\n", 
        //        viewport[2], viewport[3], viewport[0], viewport[1], previousFramebuffer);

        // Pass 1: Render shadow maps
        for (size_t i = 0; i < m_lights.size() && i < 4; ++i)
        {
            auto light = m_lights[i];
            if (!light || !light->isEnabled) continue;

            auto lightGO = light->GetOwner();
            if (!lightGO || !lightGO->transform) continue;

            lightData.positions[i] = glm::vec4(lightGO->transform->position, 1.0f);
            lightData.directions[i] = glm::vec4(lightGO->transform->forward(), 0.0f);
            
            glm::vec4 lightColor = light->GetColor();
            lightColor.w = light->intensity.Get(); // Store intensity in alpha channel
            lightData.colors[i] = lightColor;
            
            lightData.lightTypes[i] = glm::ivec4(ToInt(light->lightType.Get()), 0, 0, 0);

            // printf("[Render] Rendering shadow map for light %zu (type: %d)\n", i, ToInt(light->lightType.Get()));
            RenderShadowMap(i);
        }

        // Restore viewport AND framebuffer
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glBindFramebuffer(GL_FRAMEBUFFER, previousFramebuffer);
        // printf("[Render] Restored viewport: %d x %d, Framebuffer: %d\n", 
        //        viewport[2], viewport[3], previousFramebuffer);

        // Upload light data to UBO
        glBindBuffer(GL_UNIFORM_BUFFER, m_uboLights);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(LightData), &lightData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Check for OpenGL errors before final render
        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            printf("[ERROR] OpenGL error before RenderFinalScene: 0x%x\n", err);
        }

        // Pass 2: Render final scene
        // printf("[Render] Starting RenderFinalScene\n");
        RenderFinalScene(view, projection);

        // Check for OpenGL errors after render
        err = glGetError();
        if (err != GL_NO_ERROR)
        {
            printf("[ERROR] OpenGL error after RenderFinalScene: 0x%x\n", err);
        }

        // printf("=== Scene::Render END ===\n\n");
    }

    void Scene::RenderShadowMap(int lightIndex)
    {
        // printf("  [ShadowMap] Rendering shadow map for light %d\n", lightIndex);
        
        if (lightIndex >= m_lights.size()) return;
        auto light = m_lights[lightIndex];
        if (!light || !light->isEnabled) return;

        auto lightGO = light->GetOwner();
        if (!lightGO || !lightGO->transform) return;

        glm::mat4 lightProjection, lightView, lightSpaceMatrix;
        float near_plane = 1.0f, far_plane = 25.0f;

        if (light->lightType.Get() == LightType::Directional)
        {
            lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
            glm::vec3 lightDir = lightGO->transform->forward();
            glm::vec3 lightPos = -lightDir * 10.0f;
            lightView = glm::lookAt(lightPos, lightPos + lightDir, glm::vec3(0.0f, 1.0f, 0.0f));
            // printf("  [ShadowMap] Using directional light projection\n");
        }
        else
        {
            lightProjection = glm::perspective(glm::radians(90.0f), 1.0f, near_plane, far_plane);
            lightView = glm::lookAt(lightGO->transform->position,
                                    lightGO->transform->position + lightGO->transform->forward(),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
            // printf("  [ShadowMap] Using perspective projection\n");
        }

        lightSpaceMatrix = lightProjection * lightView;

        // Render scene from light's point of view
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBOs[lightIndex]);
        
        // Check framebuffer status
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            printf("  [ERROR] Shadow map framebuffer incomplete! Status: 0x%x\n", status);
        }
        
        glClear(GL_DEPTH_BUFFER_BIT);

        glCullFace(GL_FRONT);

        // int renderedCount = 0;
        for (const auto& renderer : m_renderers)
        {
            if (!renderer) continue;
            auto go = renderer->GetOwner();
            if (!go || !go->isEnabled || !renderer->isEnabled) continue;
            glm::mat4 modelMatrix = CalculateWorldMatrix(go);
            depthShader.setMat4("modelMatrix", modelMatrix);

            for (auto& mesh : renderer->GetMeshes())
            {
                mesh.Render(GL_TRIANGLES);
                // renderedCount++;
            }
        }
        // printf("  [ShadowMap] Rendered %d meshes to shadow map\n", renderedCount);

        glCullFace(GL_BACK);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        m_lightSpaceMatrices[lightIndex] = lightSpaceMatrix;
    }

    void Scene::RenderFinalScene(const glm::mat4& view, const glm::mat4& projection)
    {
        // Rendering all renderers
        for (const auto& renderer : m_renderers)
        {
            if (!renderer) continue;
            
            auto go = renderer->GetOwner();
            if (!go) continue;
            
            if (!go->isEnabled) continue;
            if (!renderer->isEnabled) continue;

            glm::mat4 worldMatrix = CalculateWorldMatrix(go);
            glm::mat4 mvp = projection * view * worldMatrix;

            auto material = renderer->GetMaterial();
            if (!material) continue;

            // Set matrices
            material->SetMat4("mvpMatrix", mvp);
            material->SetMat4("modelMatrix", worldMatrix);
            
            // Set bloom threshold for materials that use it
            material->SetFloat("bloomThreshold", m_bloomThreshold);
            
            // Pass light space matrix and shadow map only if we have lights
            if (!m_lightSpaceMatrices.empty() && !m_depthMaps.empty())
            {
                material->SetMat4("lightSpaceMatrix", m_lightSpaceMatrices[0]);
            }
            
            // Check OpenGL error before rendering
            GLenum err = glGetError();
            if (err != GL_NO_ERROR)
            {
                printf("  [ERROR] OpenGL error before rendering %s: 0x%x\n", go->name.c_str(), err);
            }
            
            // Call material->Use() which will bind textures and set uniforms
            material->Use();
            
            // IMPORTANT: Bind shadow map AFTER Material::Use() because Material::Use() binds its textures
            if (!m_depthMaps.empty())
            {
                glActiveTexture(GL_TEXTURE3);
                glBindTexture(GL_TEXTURE_2D, m_depthMaps[0]);
                
                // Set the uniform to point to texture unit 3
                GLint shadowMapLoc = glGetUniformLocation(material->GetShaderProgram(), "shadowMap");
                if (shadowMapLoc != -1)
                {
                    glUniform1i(shadowMapLoc, 3);
                }
            }
            
            // Now render the meshes
            for (auto& mesh : renderer->GetMeshes())
            {
                mesh.Render(GL_TRIANGLES);
            }
            
            // Check OpenGL error after rendering
            err = glGetError();
            if (err != GL_NO_ERROR)
            {
                printf("  [ERROR] OpenGL error after rendering %s: 0x%x\n", go->name.c_str(), err);
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

    void Scene::GenerateDepthMaps(int numLights, int width_resolution, int height_resolution)
    {
        // printf("[GenerateDepthMaps] Creating %d depth maps (%dx%d)\n", numLights, width_resolution, height_resolution);
        
        // Clean up old maps if they exist
        if (!m_depthMapFBOs.empty()) {
            // printf("[GenerateDepthMaps] Cleaning up old depth maps\n");
            glDeleteFramebuffers(m_depthMapFBOs.size(), m_depthMapFBOs.data());
            glDeleteTextures(m_depthMaps.size(), m_depthMaps.data());
            m_depthMapFBOs.clear();
            m_depthMaps.clear();
        }

        // Resize vectors to hold the number of lights
        m_depthMapFBOs.resize(numLights);
        m_depthMaps.resize(numLights);
        m_lightSpaceMatrices.resize(numLights);

        // Generate framebuffers and textures
        glGenFramebuffers(numLights, m_depthMapFBOs.data());
        glGenTextures(numLights, m_depthMaps.data());

        for (int i = 0; i < numLights; ++i)
        {
            // printf("[GenerateDepthMaps] Setting up depth map %d: FBO=%d, Texture=%d\n", 
            //        i, m_depthMapFBOs[i], m_depthMaps[i]);
            
            // Configure depth texture
            glBindTexture(GL_TEXTURE_2D, m_depthMaps[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                width_resolution, height_resolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
            glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

            // Attach depth texture to framebuffer
            glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBOs[i]);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthMaps[i], 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);

            // Check framebuffer completeness
            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE)
            {
                printf("[ERROR] Framebuffer %d not complete! Status: 0x%x\n", i, status);
            }
            // else
            // {
            //     printf("[GenerateDepthMaps] Framebuffer %d complete\n", i);
            // }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // printf("[GenerateDepthMaps] Depth maps created successfully\n");
    }
}
