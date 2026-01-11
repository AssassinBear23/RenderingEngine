#include "core/assimpLoader.h"
#include "core/material.h"
#include "core/model.h"
#include "core/objectSystems/components/Light.h"
#include "core/objectSystems/components/Renderer.h"
#include "core/rendering/mesh.h"
#include "core/rendering/texture.h"
#include "core/sceneManager.h"
#include "Editor.h"
#include "inputManager.h"
#include "panel.h"
#include "panels/hierarchyPanel.h"
#include "panels/inspectorPanel.h"
#include "panels/postProcessingPanel.h"
#include "panels/ViewportPanel.h"
#include <core/camera.h>
#include <core/rendering/frameBuffer.h>
#include <cstdio>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

namespace editor
{
    // Define the static member
    EditorContext Editor::editorCtx;

    Editor::~Editor() 
    { 
        if (m_initialized) 
            shutdown(); 
    }

    static void APIENTRY glDebugOutput(GLenum source,
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

    bool Editor::init(const char* glsl_version)
    {
        // Initialize GLFW
        if (!glfwInit())
        {
            printf("[EDITOR] Failed to initialize GLFW\n");
            return false;
        }

        glfwWindowHint(GLFW_SAMPLES, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

        m_window = glfwCreateWindow(800, 600, "FinalEngine Editor", nullptr, nullptr);
        if (m_window == nullptr)
        {
            printf("[EDITOR] Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(m_window);

        // Initialize GLAD
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            printf("[EDITOR] Failed to initialize GLAD\n");
            return false;
        }

        // Setup OpenGL debug callback
        int flags; 
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        }

        // Setup OpenGL state
        glEnable(GL_DEPTH_TEST);
        glFrontFace(GL_CCW);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glm::vec4 clearColor = glm::vec4(0);
        glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

        // Initialize post-processing manager
        m_postProcessingManager = std::make_shared<core::postProcessing::PostProcessingManager>();
        m_postProcessingManager->Initialize();

        // Initialize panels
        addPanel<ViewportPanel>(*this);
        addPanel<HierarchyPanel>();
        addPanel<InspectorPanel>();
        addPanel<PostProcessingPanel>(m_postProcessingManager.get());

        // Initialize ImGui
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsDark();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowPadding = ImVec2(0, 0);
        style.FramePadding = ImVec2(4, 4);

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        // Initialize input manager
        m_inputManager = std::make_unique<InputManager>();
        m_inputManager->Initialize(m_window);

        // Initialize editor camera
        m_editorCamera = std::make_unique<core::Camera>(
            glm::vec3(0.0f, 0.0f, 10.0f), 
            glm::vec3(0.0f, 1.0f, 0.0f)
        );

        // Create scene render framebuffer
        m_sceneRenderBuffer = std::make_unique<core::FrameBuffer>(
            "SceneFBO", 
            core::FrameBufferSpecifications{
                800, 600,
                core::AttachmentType::COLOR_DEPTH,
                2
            }
        );

        // Create UBO for lights
        glGenBuffers(1, &m_uboLights);
        glBindBuffer(GL_UNIFORM_BUFFER, m_uboLights);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(core::LightData), nullptr, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uboLights);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Create scene manager
        editorCtx.sceneManager = std::make_shared<core::SceneManager>();

        // Register default scenes (these are fallback/example scenes)
        registerDefaultScenes();

        // Try to load a saved scene, if none exists, load default
        if (!tryLoadSavedScene())
        {
            loadDefaultScene();
        }

        m_initialized = true;
        printf("[EDITOR] Successfully initialized\n");
        return true;
    }

    void Editor::shutdown()
    {
        if (!m_initialized) return;

        // Cleanup UBO
        if (m_uboLights != 0)
        {
            glDeleteBuffers(1, &m_uboLights);
            m_uboLights = 0;
        }

        // Cleanup ImGui
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Cleanup GLFW
        if (m_window)
        {
            glfwDestroyWindow(m_window);
            m_window = nullptr;
        }
        glfwTerminate();

        m_initialized = false;
        printf("[EDITOR] Shutdown complete\n");
    }

    void Editor::run()
    {
        if (!m_initialized)
        {
            printf("[EDITOR] Cannot run - not initialized!\n");
            return;
        }

        m_isRunning = true;

        double currentTime = glfwGetTime();
        float deltaTime = 0.0f;

        // Main editor loop
        while (m_isRunning && !glfwWindowShouldClose(m_window))
        {
            glfwPollEvents();

            beginFrame();
            draw();

            // Render the 3D scene
            renderScene(deltaTime);

            endFrame();
            glfwSwapBuffers(m_window);

            // Calculate delta time
            double finishFrameTime = glfwGetTime();
            deltaTime = static_cast<float>(finishFrameTime - currentTime);
            currentTime = finishFrameTime;
        }

        printf("[EDITOR] Main loop ended\n");
    }

    void Editor::renderScene(float deltaTime)
    {
        auto currentScene = editorCtx.sceneManager ? editorCtx.sceneManager->GetCurrentScene() : nullptr;

        int vw = getViewportWidth();
        int vh = getViewportHeight();

        if (!m_sceneRenderBuffer)
            return;

        m_sceneRenderBuffer->Resize(vw, vh);

        core::FrameBuffer* viewportFrameBuffer = GetFrameBuffer();

        if (vw > 0 && vh > 0)
        {
            m_sceneRenderBuffer->BindAndClear(vw, vh);

            // Process input if viewport is focused
            if (viewportFocused() && m_inputManager && m_editorCamera)
                m_inputManager->ProcessInput(m_window, m_editorCamera.get(), deltaTime);

            // Render 3D scene
            if (currentScene && m_editorCamera)
            {
                glm::mat4 view = m_editorCamera->GetViewMatrix();
                glm::mat4 projection = m_editorCamera->GetProjectionMatrix(vw, vh);
                currentScene->Render(view, projection);
            }

            // Apply post-processing if effects are registered
            if (m_postProcessingManager && viewportFrameBuffer)
            {
                m_postProcessingManager->ProcessStack(
                    *m_sceneRenderBuffer, 
                    *viewportFrameBuffer,
                    vw, vh
                );
            }

            m_sceneRenderBuffer->Unbind();
        }
    }

    void Editor::beginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void Editor::drawDockspace()
    {
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoNavFocus;

        const ImGuiViewport* vp = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(vp->WorkPos);
        ImGui::SetNextWindowSize(vp->WorkSize);
        ImGui::SetNextWindowViewport(vp->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, m_windowBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, m_windowBorderSize);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, m_windowPadding);
        ImGui::Begin("DockSpaceHost", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        m_dockspaceId = ImGui::GetID("MainDockspace");
        ImGui::DockSpace(m_dockspaceId, ImVec2(0, 0), ImGuiDockNodeFlags_None);
    }

    void Editor::drawMainMenu()
    {
        if (!ImGui::BeginMainMenuBar()) return;

        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) 
            { 
                m_isRunning = false;
                glfwSetWindowShouldClose(m_window, true); 
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings"))
        {
            if (ImGui::BeginMenu("Scene"))
            {
                if (editorCtx.sceneManager)
                {
                    auto sceneNames = editorCtx.sceneManager->GetSceneNames();
                    for (const auto& sceneName : sceneNames)
                    {
                        if (ImGui::MenuItem(sceneName.c_str()))
                        {
                            editorCtx.sceneManager->LoadScene(sceneName, m_uboLights);
                        }
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Extra Options"))
            {
                ImGui::DragFloat("Font Scale", &ImGui::GetIO().FontGlobalScale, 0.01f, 0.5f, 3.0f);
                ImGui::DragFloat("Window Rounding", &m_windowRoundingValue, 0.1f, 0.0f, 12.0f);
                ImGui::DragFloat("Window Border Size", &m_windowBorderSize, 0.1f, 0.0f, 5.0f);
                ImGui::DragFloat2("Window Padding", &m_windowPadding[0], 0.1f, 0.0f, 20.0f);
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Windows"))
        {
            for (auto& p : m_panels)
            {
                ImGui::Checkbox(p->name(), &p->isVisible);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    void Editor::draw()
    {
        drawDockspace();
        drawMainMenu();

        for (auto& p : m_panels)
            if (p->isVisible)
                p->draw(editorCtx);

        ImGui::End(); // DockSpaceHost
    }

    void Editor::endFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    core::FrameBuffer* Editor::GetFrameBuffer()     const { return m_viewport ? m_viewport->GetFrameBuffer() : nullptr; }
    int                Editor::getViewportWidth()   const { return m_viewport ? m_viewport->GetWidth() : 0; }
    int                Editor::getViewportHeight()  const { return m_viewport ? m_viewport->GetHeight() : 0; }
    bool               Editor::viewportFocused()    const { return m_viewport ? m_viewport->isFocused() : false; }

    void Editor::registerDefaultScenes()
    {
        if (!editorCtx.sceneManager)
        {
            printf("[EDITOR] Cannot register default scenes - no SceneManager\n");
            return;
        }

        printf("[EDITOR] Registering default scenes...\n");

        // Load shaders for default scenes
        m_modelShader = std::make_unique<core::Shader>("assets/shaders/vertex.vert", "assets/shaders/fragment.frag");
        m_textureShader = std::make_unique<core::Shader>("assets/shaders/vertex.vert", "assets/shaders/texture.frag");
        m_lightBulbShader = std::make_unique<core::Shader>("assets/shaders/vertex.vert", "assets/shaders/fragmentLightBulb.frag");
        m_litSurfaceShader = std::make_unique<core::Shader>("assets/shaders/vertex.vert", "assets/shaders/litFragment.frag");

        // Register Default Scene 1
        editorCtx.sceneManager->RegisterScene("Default Scene 1", [this](auto scene) {
            auto rockGO = scene->CreateObject("Rock");
            core::Model rockModel = core::AssimpLoader::loadModel("assets/models/rockModel.fbx");
            auto rockMaterial = std::make_shared<core::Material>(m_litSurfaceShader->ID);

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
            core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
            auto suzanneMaterial = std::make_shared<core::Material>(m_litSurfaceShader->ID);
            suzanneMaterial->SetBool("useNormalMap", false);
            auto suzanneRenderer = suzanneGO->AddComponent<core::Renderer>();
            suzanneRenderer->SetMeshes(suzanneModel.GetMeshes());
            suzanneRenderer->SetMaterial(suzanneMaterial);

            auto quadGO = scene->CreateObject("Quad");
            quadGO->SetParent(suzanneGO);
            quadGO->transform->position = glm::vec3(0, 0, -2.5f);
            quadGO->transform->scale = glm::vec3(5, 5, 1);
            core::Mesh quadMesh = core::Mesh::GenerateQuad();
            auto quadTexture = std::make_shared<core::Texture>("assets/textures/CMGaTo_crop.png");
            auto quadMaterial = std::make_shared<core::Material>(m_textureShader->ID);
            quadMaterial->SetTexture("text", quadTexture, 0);
            auto quadRenderer = quadGO->AddComponent<core::Renderer>();
            quadRenderer->SetMesh(quadMesh);
            quadRenderer->SetMaterial(quadMaterial);

            auto lightGO = scene->CreateObject("Light");
            core::Model lightModel = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
            auto lightMaterial = std::make_shared<core::Material>(m_lightBulbShader->ID);
            auto lightRenderer = lightGO->AddComponent<core::Renderer>();
            lightRenderer->SetMeshes(lightModel.GetMeshes());
            lightRenderer->SetMaterial(lightMaterial);
            lightGO->transform->position = glm::vec3(2.0f, 2.0f, 2.0f);
            lightGO->transform->scale = glm::vec3(.1f, .1f, .1f);
            auto lightComp = lightGO->AddComponent<core::Light>();
            lightComp->color = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f);
        });

        // Register Default Scene 2
        editorCtx.sceneManager->RegisterScene("Default Scene 2", [this](auto scene) {
            auto suzanneGO = scene->CreateObject("Suzanne1");
            core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
            auto suzanneMaterial = std::make_shared<core::Material>(m_litSurfaceShader->ID);
            auto suzanneRenderer = suzanneGO->AddComponent<core::Renderer>();
            suzanneRenderer->SetMeshes(suzanneModel.GetMeshes());
            suzanneRenderer->SetMaterial(suzanneMaterial);

            auto suzanneGO2 = scene->CreateObject("Suzanne2");
            suzanneGO2->transform->position = glm::vec3(3, 0, 0);
            core::Model suzanneModel2 = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
            auto suzanneMaterial2 = std::make_shared<core::Material>(m_litSurfaceShader->ID);
            auto suzanneRenderer2 = suzanneGO2->AddComponent<core::Renderer>();
            suzanneRenderer2->SetMeshes(suzanneModel2.GetMeshes());
            suzanneRenderer2->SetMaterial(suzanneMaterial2);

            auto lightGO = scene->CreateObject("Light");
            core::Model lightModel = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
            auto lightMaterial = std::make_shared<core::Material>(m_lightBulbShader->ID);
            auto lightRenderer = lightGO->AddComponent<core::Renderer>();
            lightRenderer->SetMeshes(lightModel.GetMeshes());
            lightRenderer->SetMaterial(lightMaterial);
            lightGO->transform->position = glm::vec3(2.0f, 2.0f, 2.0f);
            lightGO->transform->scale = glm::vec3(.1f, .1f, .1f);
            auto lightComp = lightGO->AddComponent<core::Light>();
            lightComp->color = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f);

            auto lightGO2 = scene->CreateObject("Light2");
            core::Model lightModel2 = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
            auto lightMaterial2 = std::make_shared<core::Material>(m_lightBulbShader->ID);
            auto lightRenderer2 = lightGO2->AddComponent<core::Renderer>();
            lightRenderer2->SetMeshes(lightModel2.GetMeshes());
            lightRenderer2->SetMaterial(lightMaterial2);
            lightGO2->transform->position = glm::vec3(-2.0f, 0, -2.0f);
            lightGO2->transform->scale = glm::vec3(.1f, .1f, .1f);
            auto lightComp2 = lightGO2->AddComponent<core::Light>();
            lightComp2->color = glm::vec4(0.2f, 0.8f, 1.0f, 1.0f);
        });

        printf("[EDITOR] Default scenes registered\n");
    }

    bool Editor::tryLoadSavedScene()
    {
        // TODO: When you implement serialization, check for saved scene file here
        // For now, always return false to load default scenes
        
        // Example future implementation:
        // if (std::filesystem::exists("saved_scene.json")) {
        //     return deserializeAndLoadScene("saved_scene.json");
        // }
        
        return false;
    }

    void Editor::loadDefaultScene()
    {
        if (!editorCtx.sceneManager)
        {
            printf("[EDITOR] Cannot load default scene - no SceneManager\n");
            return;
        }

        // Try to load the first default scene
        auto sceneNames = editorCtx.sceneManager->GetSceneNames();
        if (!sceneNames.empty())
        {
            printf("[EDITOR] Loading default scene: %s\n", sceneNames[0].c_str());
            editorCtx.sceneManager->LoadScene(sceneNames[0], m_uboLights);
        }
        else
        {
            printf("[EDITOR] WARNING: No scenes registered!\n");
        }
    }
}