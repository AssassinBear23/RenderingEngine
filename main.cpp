#include "core/assimpLoader.h"
#include "core/camera.h"
#include "core/material.h"
#include "core/model.h"
#include "core/objectSystems/components/Light.h"
#include "core/objectSystems/components/Renderer.h"
#include "core/rendering/mesh.h"
#include "core/rendering/texture.h"
#include "core/scene.h"
#include "core/sceneManager.h"
#include "editor/Editor.h"
#include <cstdio>
#include <editor/inputManager.h>
#include <editor/panels/hierarchyPanel.h>
#include <editor/panels/inspectorPanel.h>
#include <editor/panels/viewportPanel.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <unordered_map>
#include <core/Rendering/shader.h>
using namespace editor;

int g_width = 800;
int g_height = 600;
std::unordered_map<int, bool> g_keymap;
std::unique_ptr<core::Camera> editorCamera;

static bool g_rotating = false;
static bool g_firstMouse = true;
static double g_lastX = 0.0;
static double g_lastY = 0.0;
static double g_mouseDeltaX = 0.0;
static double g_mouseDeltaY = 0.0;

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(g_width, g_height, "FinalEngine", nullptr, nullptr);
    if (window == nullptr)
    {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    Editor editor;
    editor.init(window, "#version 400");

    editor.addPanel<ViewportPanel>(editor);
    editor.addPanel<HierarchyPanel>();
    editor.addPanel<InspectorPanel>();

    InputManager inputManager;
    inputManager.Initialize(window);

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    core::Shader      modelShader("assets/shaders/vertex.vert", "assets/shaders/fragment.frag");
    core::Shader    textureShader("assets/shaders/vertex.vert", "assets/shaders/texture.frag");
    core::Shader  lightBulbShader("assets/shaders/vertex.vert", "assets/shaders/fragmentLightBulb.frag");
    core::Shader litSurfaceShader("assets/shaders/vertex.vert", "assets/shaders/litFragment.frag");

    // Create UBO for lights
    GLuint uboLights;
    glGenBuffers(1, &uboLights);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(core::LightData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboLights);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    auto sceneManager = std::make_shared<core::SceneManager>();
    Editor::editorCtx.sceneManager = sceneManager;

    auto postProcessingManager = std::make_shared<core::postProcessing::PostProcessingManager>();

    // Create Scene 1
    sceneManager->RegisterScene("Scene 1", [&modelShader, &textureShader, &lightBulbShader, &litSurfaceShader](auto scene) {
        // Create Suzanne GameObject
        auto rockGO = scene->CreateObject("Rock");

        core::Model rockModel = core::AssimpLoader::loadModel("assets/models/rockModel.fbx");
        auto rockMaterial = std::make_shared<core::Material>(litSurfaceShader.ID);

        auto rockRenderer = rockGO->AddComponent<core::Renderer>();
        auto rockTexture = std::make_shared<core::Texture>("assets/textures/rockTexture.jpeg");
        auto rockAO = std::make_shared<core::Texture>("assets/textures/rockAO.jpeg");
        auto rockNormal = std::make_shared<core::Texture>("assets/textures/rockNormal.jpeg");
        rockMaterial->SetTexture("albedoMap", rockTexture, 0);
        rockMaterial->SetTexture("aoMap", rockAO, 1);
        rockMaterial->SetTexture("normalMap", rockNormal, 2);
        rockMaterial->SetBool("useNormalMap", true);
        rockRenderer->SetMeshes(rockModel.GetMeshes());
        rockRenderer->SetMaterial(rockMaterial);

        rockGO->transform->rotation = glm::vec3(-90, 0, 0);
        rockGO->transform->scale = glm::vec3(0.3f, 0.3f, 0.3f);
        
        auto suzanneGO = scene->CreateObject("Suzanne");

        // Load Suzanne model and create material
        core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial = std::make_shared<core::Material>(litSurfaceShader.ID);
        suzanneMaterial->SetBool("useNormalMap", false);

        auto suzanneRenderer = suzanneGO->AddComponent<core::Renderer>();
        suzanneRenderer->SetMeshes(suzanneModel.GetMeshes());
        suzanneRenderer->SetMaterial(suzanneMaterial);

        // Create Quad GameObject
        auto quadGO = scene->CreateObject("Quad");
        quadGO->SetParent(suzanneGO);
        quadGO->transform->position = glm::vec3(0, 0, -2.5f);
        quadGO->transform->scale = glm::vec3(5, 5, 1);

        // Create quad mesh and material with texture
        core::Mesh quadMesh = core::Mesh::GenerateQuad();
        auto quadTexture = std::make_shared<core::Texture>("assets/textures/CMGaTo_crop.png");
        auto quadMaterial = std::make_shared<core::Material>(textureShader.ID);
        quadMaterial->SetTexture("text", quadTexture, 0);

        auto quadRenderer = quadGO->AddComponent<core::Renderer>();
        quadRenderer->SetMesh(quadMesh);
        quadRenderer->SetMaterial(quadMaterial);

        auto lightGO = scene->CreateObject("Light");

        // Load model and create material with lightbulb shader
        core::Model lightModel = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
        auto lightMaterial = std::make_shared<core::Material>(lightBulbShader.ID);

        // Add Renderer FIRST (before Light)
        auto lightRenderer = lightGO->AddComponent<core::Renderer>();
        lightRenderer->SetMeshes(lightModel.GetMeshes());
        lightRenderer->SetMaterial(lightMaterial);

        lightGO->transform->position = glm::vec3(2.0f, 2.0f, 2.0f);
        lightGO->transform->scale = glm::vec3(.1f, .1f, .1f);

        auto lightComp = lightGO->AddComponent<core::Light>();
        lightComp->color = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f); // Orange color

        return scene;
        });

    // Create Scene 2
    sceneManager->RegisterScene("Scene 2", [&modelShader, &litSurfaceShader, &lightBulbShader](auto scene) {
        // Create Suzanne GameObject
        auto suzanneGO = scene->CreateObject("Suzanne1");

        // Load Suzanne model and create material
        core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial = std::make_shared<core::Material>(litSurfaceShader.ID);

        auto suzanneRenderer = suzanneGO->AddComponent<core::Renderer>();
        suzanneRenderer->SetMeshes(suzanneModel.GetMeshes());
        suzanneRenderer->SetMaterial(suzanneMaterial);

        // Create Suzanne2 GameObject
        auto suzanneGO2 = scene->CreateObject("Suzanne2");
        suzanneGO2->transform->position = glm::vec3(3, 0, 0);

        // Load Suzanne model and create material
        core::Model suzanneModel2 = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial2 = std::make_shared<core::Material>(litSurfaceShader.ID);

        auto suzanneRenderer2 = suzanneGO2->AddComponent<core::Renderer>();
        suzanneRenderer2->SetMeshes(suzanneModel2.GetMeshes());
        suzanneRenderer2->SetMaterial(suzanneMaterial2);

        auto lightGO = scene->CreateObject("Light");

        // Load model and create material with lightbulb shader
        core::Model lightModel = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
        auto lightMaterial = std::make_shared<core::Material>(lightBulbShader.ID);

        // Add Renderer FIRST (before Light)
        auto lightRenderer = lightGO->AddComponent<core::Renderer>();
        lightRenderer->SetMeshes(lightModel.GetMeshes());
        lightRenderer->SetMaterial(lightMaterial);

        lightGO->transform->position = glm::vec3(2.0f, 2.0f, 2.0f);
        lightGO->transform->scale = glm::vec3(.1f, .1f, .1f);

        auto lightComp = lightGO->AddComponent<core::Light>();
        lightComp->color = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f); // Orange color

        auto lightGO2 = scene->CreateObject("Light2");

        // Load model and create material with lightbulb shader
        core::Model lightModel2 = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
        auto lightMaterial2 = std::make_shared<core::Material>(lightBulbShader.ID);

        // Add Renderer FIRST (before Light)
        auto lightRenderer2 = lightGO2->AddComponent<core::Renderer>();
        lightRenderer2->SetMeshes(lightModel2.GetMeshes());
        lightRenderer2->SetMaterial(lightMaterial2);

        lightGO2->transform->position = glm::vec3(-2.0f, 0, -2.0f);
        lightGO2->transform->scale = glm::vec3(.1f, .1f, .1f);

        auto lightComp2 = lightGO2->AddComponent<core::Light>();
        lightComp2->color = glm::vec4(0.2f, 0.8f, 1.0f, 1.0f); // Orange color

        return scene;
        });

    sceneManager->LoadScene("Scene 1", uboLights);

    glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

    editorCamera = std::make_unique<core::Camera>(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    double currentTime = glfwGetTime();
    double finishFrameTime = 0.0;
    float deltaTime = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        editor.beginFrame();
        editor.draw();

        auto currentScene = sceneManager->GetCurrentScene();

        GLuint fb = editor.framebuffer();
        int vw = editor.getViewportWidth();
        int vh = editor.getViewportHeight();
        if (vw <= 0 || vh <= 0 || editor.framebuffer() == 0)
        {
            // Skip 3D rendering this frame
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, fb);
            glViewport(0, 0, vw, vh);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (editor.viewportFocused())
                inputManager.ProcessInput(window, editorCamera.get(), deltaTime);

            // Render 3D scene
            glm::mat4 view = editorCamera->GetViewMatrix();
            glm::mat4 projection = editorCamera->GetProjectionMatrix(
                static_cast<float>(vw),
                static_cast<float>(vh)
            );

            if (currentScene)
                currentScene->Render(view, projection);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        editor.endFrame();
        glfwSwapBuffers(window);

        finishFrameTime = glfwGetTime();
        deltaTime = static_cast<float>(finishFrameTime - currentTime);
        currentTime = finishFrameTime;
    }

    editor.shutdown();
    glfwTerminate();
    return 0;
}
