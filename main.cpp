#include "core/assimpLoader.h"
#include "core/camera.h"
#include "core/material.h"
#include "core/model.h"
#include "core/objectSystems/components/Renderer.h"
#include "core/objectSystems/components/Light.h"
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
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <memory>
#include <sstream>
#include <string>
#include <regex>
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

std::string ReadFileToString(const std::string& filePath);

std::string ProcessShaderIncludes(const std::string& source, const std::string& basePath = "assets/shaders/shaderlibrary/")
{
    std::string result = source;
    std::regex includePattern(R"(#include\s+\"([^\"]+)\")");
    std::smatch match;

    // Keep processing until no more includes found (supports nested includes)
    while (std::regex_search(result, match, includePattern))
    {
        std::string includeFile = match[1].str();
        std::string includePath = basePath + includeFile;

        printf("Processing #include \"%s\" from %s\n", includeFile.c_str(), includePath.c_str());

        std::string includeContent = ReadFileToString(includePath);

        if (includeContent.empty())
        {
            printf("Warning: Could not read include file: %s\n", includePath.c_str());
        }
        else
        {
            printf("Successfully loaded include: %s (%zu bytes)\n", includeFile.c_str(), includeContent.size());
        }

        // Replace the #include directive with the file content
        result = std::regex_replace(result, includePattern, includeContent, std::regex_constants::format_first_only);
    }

    return result;
}

std::string ReadFileToString(const std::string& filePath)
{
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open())
    {
        printf("Could not open file: %s\n", filePath.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return ProcessShaderIncludes(buffer.str());
}


GLuint GenerateShader(const std::string& shaderPath, GLuint shaderType)
{
    printf("Loading shader: %s\n", shaderPath.c_str());
    const std::string shaderText = ReadFileToString(shaderPath);
    const GLuint shader = glCreateShader(shaderType);
    const char* s_str = shaderText.c_str();
    glShaderSource(shader, 1, &s_str, nullptr);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        printf("Error! Shader issue [%s]: %s\n", shaderPath.c_str(), infoLog);
    }
    return shader;
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
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
    // editor.addPanel<TransformPanel>(); // No longer needed due to InspectorPanel functioning
    editor.addPanel<InspectorPanel>();

    InputManager inputManager;
    inputManager.Initialize(window);

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shaders
    const GLuint vertexShader = GenerateShader("assets/shaders/vertex.vert", GL_VERTEX_SHADER);
    const GLuint fragmentShader = GenerateShader("assets/shaders/fragment.frag", GL_FRAGMENT_SHADER);
    const GLuint textureShader = GenerateShader("assets/shaders/texture.frag", GL_FRAGMENT_SHADER);
    const GLuint lightBulbShader = GenerateShader("assets/shaders/fragmentLightBulb.frag", GL_FRAGMENT_SHADER);
    const GLuint litSurfaceShader = GenerateShader("assets/shaders/litFragment.frag", GL_FRAGMENT_SHADER);

    int success;
    char infoLog[512];


    // Model shader program (for Suzanne)
    const unsigned int modelShaderProgram = glCreateProgram();
    glAttachShader(modelShaderProgram, vertexShader);
    glAttachShader(modelShaderProgram, fragmentShader);
    glLinkProgram(modelShaderProgram);
    glGetProgramiv(modelShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(modelShaderProgram, 512, nullptr, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }

    // Texture shader program (for Quad)
    const unsigned int textureShaderProgram = glCreateProgram();
    glAttachShader(textureShaderProgram, vertexShader);
    glAttachShader(textureShaderProgram, textureShader);
    glLinkProgram(textureShaderProgram);
    glGetProgramiv(textureShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(textureShaderProgram, 512, nullptr, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }

    // Lightbulb shader program (for light gizmos)
    const unsigned int lightBulbShaderProgram = glCreateProgram();
    glAttachShader(lightBulbShaderProgram, vertexShader);
    glAttachShader(lightBulbShaderProgram, lightBulbShader);
    glLinkProgram(lightBulbShaderProgram);
    glGetProgramiv(lightBulbShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(lightBulbShaderProgram, 512, nullptr, infoLog);
        printf("Error! Making Lightbulb Shader Program: %s\n", infoLog);
    }

    const unsigned int litSurfaceProgram = glCreateProgram();
    glAttachShader(litSurfaceProgram, vertexShader);
    glAttachShader(litSurfaceProgram, litSurfaceShader);
    glLinkProgram(litSurfaceProgram);
    glGetProgramiv(litSurfaceProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(litSurfaceProgram, 512, nullptr, infoLog);
        printf("Error! Making Lit Surface Shader Program: %s\n", infoLog);
    }

    // Create UBO for lights
    GLuint uboLights;
    glGenBuffers(1, &uboLights);
    glBindBuffer(GL_UNIFORM_BUFFER, uboLights);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(core::LightData), nullptr, GL_DYNAMIC_DRAW);

    // Bind to binding point 0
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, uboLights);

    // Link shader programs to the same binding point
    GLuint lightBlockIndex = glGetUniformBlockIndex(modelShaderProgram, "LightBlock");
    glUniformBlockBinding(modelShaderProgram, lightBlockIndex, 0);

    lightBlockIndex = glGetUniformBlockIndex(textureShaderProgram, "LightBlock");
    glUniformBlockBinding(textureShaderProgram, lightBlockIndex, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, 0); // Unbind

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(textureShader);
    glDeleteShader(lightBulbShader);
    glDeleteShader(litSurfaceShader);

    auto sceneManager = std::make_shared<core::SceneManager>();
    Editor::editorCtx.sceneManager = sceneManager;

    // Create Scene 1
    sceneManager->RegisterScene("Scene 1", [modelShaderProgram, textureShaderProgram, lightBulbShaderProgram, litSurfaceProgram](auto scene) {
        // Create Suzanne GameObject
        auto rockGO = scene->CreateObject("Rock");

        core::Model rockModel = core::AssimpLoader::loadModel("assets/models/rockModel.fbx");
        auto rockMaterial = std::make_shared<core::Material>(litSurfaceProgram);

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
        auto suzanneMaterial = std::make_shared<core::Material>(litSurfaceProgram);
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
        auto quadMaterial = std::make_shared<core::Material>(textureShaderProgram);
        quadMaterial->SetTexture("text", quadTexture, 0);

        auto quadRenderer = quadGO->AddComponent<core::Renderer>();
        quadRenderer->SetMesh(quadMesh);
        quadRenderer->SetMaterial(quadMaterial);

        auto lightGO = scene->CreateObject("Light");

        // Load model and create material with lightbulb shader
        core::Model lightModel = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
        auto lightMaterial = std::make_shared<core::Material>(lightBulbShaderProgram);

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
    sceneManager->RegisterScene("Scene 2", [modelShaderProgram, litSurfaceProgram, lightBulbShaderProgram](auto scene) {
        // Create Suzanne GameObject
        auto suzanneGO = scene->CreateObject("Suzanne1");

        // Load Suzanne model and create material
        core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial = std::make_shared<core::Material>(litSurfaceProgram);

        auto suzanneRenderer = suzanneGO->AddComponent<core::Renderer>();
        suzanneRenderer->SetMeshes(suzanneModel.GetMeshes());
        suzanneRenderer->SetMaterial(suzanneMaterial);

        // Create Suzanne2 GameObject
        auto suzanneGO2 = scene->CreateObject("Suzanne2");
        suzanneGO2->transform->position = glm::vec3(3, 0, 0);

        // Load Suzanne model and create material
        core::Model suzanneModel2 = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial2 = std::make_shared<core::Material>(litSurfaceProgram);

        auto suzanneRenderer2 = suzanneGO2->AddComponent<core::Renderer>();
        suzanneRenderer2->SetMeshes(suzanneModel2.GetMeshes());
        suzanneRenderer2->SetMaterial(suzanneMaterial2);

        auto lightGO = scene->CreateObject("Light");

        // Load model and create material with lightbulb shader
        core::Model lightModel = core::AssimpLoader::loadModel("assets/models/lightBulbModel.obj");
        auto lightMaterial = std::make_shared<core::Material>(lightBulbShaderProgram);

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
        auto lightMaterial2 = std::make_shared<core::Material>(lightBulbShaderProgram);

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

    glDeleteProgram(modelShaderProgram);
    glDeleteProgram(textureShaderProgram);
    glDeleteProgram(lightBulbShaderProgram);
    glDeleteProgram(litSurfaceProgram);
    glfwTerminate();
    return 0;
}
