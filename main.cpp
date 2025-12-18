#include "core/assimpLoader.h"
#include "core/camera.h"
#include "core/material.h"
#include "core/model.h"
#include "core/objectSystems/components/Light.h"
#include "core/objectSystems/components/Renderer.h"
#include "core/rendering/frameBuffer.h"
#include "core/rendering/mesh.h"
#include "core/rendering/postProcessing/postProcessingManager.h"
#include "core/rendering/shader.h"
#include "core/rendering/texture.h"
#include "core/scene.h"
#include "core/sceneManager.h"
#include "editor/Editor.h"
#include "editor/inputManager.h"
#include "editor/panels/hierarchyPanel.h"
#include "editor/panels/inspectorPanel.h"
#include "editor/panels/postProcessingPanel.h"
#include "editor/panels/viewportPanel.h"
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <unordered_map>
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

void APIENTRY glDebugOutput(GLenum source,
                            GLenum type,
                            unsigned int id,
                            GLenum severity,
                            GLsizei length,
                            const char* message,
                            const void* userParam)
{
    // ignore non-significant error/warning codes
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;

    // Break on errors and warnings in debug builds
    if (type == GL_DEBUG_TYPE_ERROR || type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR)
    {
        //__debugbreak();  // MSVC debugger break - inspect call stack here!
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

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

    int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }

    auto postProcessingManager = std::make_shared<core::postProcessing::PostProcessingManager>();
    postProcessingManager->Initialize();
    
    Editor editor;
    editor.init(window, "#version 430");

    editor.addPanel<ViewportPanel>(editor);
    editor.addPanel<HierarchyPanel>();
    editor.addPanel<InspectorPanel>();
    editor.addPanel<PostProcessingPanel>(postProcessingManager.get());

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

    core::FrameBuffer sceneRender("SceneFBO", core::FrameBufferSpecifications{
            static_cast<unsigned int>(g_width),
            static_cast<unsigned int>(g_height),
            core::AttachmentType::COLOR_DEPTH,
            2
        });

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        editor.beginFrame();
        editor.draw();

        auto currentScene = sceneManager->GetCurrentScene();

        int vw = editor.getViewportWidth();
        int vh = editor.getViewportHeight();

        sceneRender.Resize(vw, vh);

        core::FrameBuffer* viewportFrameBuffer = editor.GetFrameBuffer();

        if (vw > 0 && vh > 0)
        {
            sceneRender.BindAndClear(vw, vh);

            if (editor.viewportFocused())
                inputManager.ProcessInput(window, editorCamera.get(), deltaTime);

            // Render 3D scene
            glm::mat4 view = editorCamera->GetViewMatrix();
            glm::mat4 projection = editorCamera->GetProjectionMatrix(vw, vh);

            if (currentScene)
                currentScene->Render(view, projection);

            // Apply post-processing if effects are registered
            if (postProcessingManager && viewportFrameBuffer)
            {
                // Process back to viewport
                postProcessingManager->ProcessStack(
                    sceneRender, *viewportFrameBuffer,
                    vw, vh);
            }

            sceneRender.Unbind();
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
