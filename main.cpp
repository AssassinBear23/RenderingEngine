#include "core/assimpLoader.h"
#include "core/camera.h"
#include "core/material.h"
#include "core/objectSystems/components/renderer.h"
#include "core/rendering/mesh.h"
#include "core/rendering/texture.h"
#include "core/sceneManager.h"
#include "editor/Editor.h"
#include <editor/inputManager.h>
#include <editor/panels/hierarchyPanel.h>
#include <editor/panels/transformPanel.h>
#include <editor/panels/viewportPanel.h>
#include <fstream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <sstream>
#include <unordered_map>

#define VSTUDIO
#ifdef VSTUDIO
#include <imgui.h>
#endif
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
    return buffer.str();
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

    auto& viewport = editor.addPanel<ViewportPanel>(editor);
    editor.addPanel<HierarchyPanel>();
    editor.addPanel<TransformPanel>();

    InputManager inputManager;
    inputManager.Initialize(window);

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Create shaders
    const GLuint modelVertexShader = GenerateShader("assets/shaders/modelVertex.vs", GL_VERTEX_SHADER);
    const GLuint fragmentShader = GenerateShader("assets/shaders/fragment.fs", GL_FRAGMENT_SHADER);
    const GLuint textureShader = GenerateShader("assets/shaders/texture.fs", GL_FRAGMENT_SHADER);

    int success;
    char infoLog[512];

    // Model shader program (for Suzanne)
    const unsigned int modelShaderProgram = glCreateProgram();
    glAttachShader(modelShaderProgram, modelVertexShader);
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
    glAttachShader(textureShaderProgram, modelVertexShader);
    glAttachShader(textureShaderProgram, textureShader);
    glLinkProgram(textureShaderProgram);
    glGetProgramiv(textureShaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(textureShaderProgram, 512, nullptr, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }

    glDeleteShader(modelVertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(textureShader);

    auto sceneManager = std::make_shared<core::SceneManager>();
    Editor::editorCtx.sceneManager = sceneManager;

    // Create Scene 1
    sceneManager->RegisterScene("Scene 1", [modelShaderProgram, textureShaderProgram](auto scene) {
        // Create Suzanne GameObject
        auto suzanneGO = scene->CreateObject("Suzanne");

        // Load Suzanne model and create material
        core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial = std::make_shared<core::Material>(modelShaderProgram);

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

        return scene;
                                });

    sceneManager->RegisterScene("Scene 2", [modelShaderProgram, textureShaderProgram](auto scene) {
        // Create Suzanne GameObject
        auto suzanneGO = scene->CreateObject("Suzanne1");

        // Load Suzanne model and create material
        core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial = std::make_shared<core::Material>(modelShaderProgram);

        auto suzanneRenderer = suzanneGO->AddComponent<core::Renderer>();
        suzanneRenderer->SetMeshes(suzanneModel.GetMeshes());
        suzanneRenderer->SetMaterial(suzanneMaterial);

        // Create Suzanne GameObject
        auto suzanneGO2 = scene->CreateObject("Suzanne2");

        // Load Suzanne model and create material
        core::Model suzanneModel2 = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
        auto suzanneMaterial2 = std::make_shared<core::Material>(modelShaderProgram);

        auto suzanneRenderer2 = suzanneGO2->AddComponent<core::Renderer>();
        suzanneRenderer2->SetMeshes(suzanneModel2.GetMeshes());
        suzanneRenderer2->SetMaterial(suzanneMaterial2);

        return scene;
                                });

    glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

    editorCamera = std::make_unique<core::Camera>(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    double currentTime = glfwGetTime();
    double finishFrameTime = 0.0;
    float deltaTime = 0.0f;

    ImGuiIO& io = ImGui::GetIO();

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
            glBindFramebuffer(GL_FRAMEBUFFER, editor.framebuffer());
            glViewport(0, 0, editor.getViewportWidth(), editor.getViewportHeight());
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render 3D scene
            glm::mat4 view = editorCamera->GetViewMatrix();
            glm::mat4 projection = editorCamera->GetProjectionMatrix(
                static_cast<float>(editor.getViewportWidth()),
                static_cast<float>(editor.getViewportHeight())
            );

            if (editor.viewportFocused())
                inputManager.ProcessInput(window, editorCamera.get(), deltaTime);

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
    glfwTerminate();
    return 0;
}
