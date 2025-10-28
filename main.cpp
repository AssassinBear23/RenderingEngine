#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "Rendering/mesh.h"
#include "core/assimpLoader.h"
#include "Rendering/texture.h"
#include "core/camera.h"

#define VSTUDIO

#ifdef VSTUDIO
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif

int g_width = 800;
int g_height = 600;
std::unordered_map<int, bool> g_keymap;
std::unique_ptr<core::Camera> editorCamera;

static bool g_rotating = false;
static bool g_firstMouse = true;
static double g_lastX = 0.0;
static double g_lastY = 0.0;


void FramebufferSizeCallback(GLFWwindow* window,
    int width, int height) {
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
}

std::string ReadFileToString(const std::string& filePath) {
    std::ifstream fileStream(filePath, std::ios::in);
    if (!fileStream.is_open()) {
        printf("Could not open file: %s\n", filePath.c_str());
        return "";
    }
    std::stringstream buffer;
    buffer << fileStream.rdbuf();
    return buffer.str();
}

GLuint GenerateShader(const std::string& shaderPath, GLuint shaderType) {
    printf("Loading shader: %s\n", shaderPath.c_str());
    const std::string shaderText = ReadFileToString(shaderPath);
    const GLuint shader = glCreateShader(shaderType);
    const char* s_str = shaderText.c_str();
    glShaderSource(shader, 1, &s_str, nullptr);
    glCompileShader(shader);
    GLint success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        printf("Error! Shader issue [%s]: %s\n", shaderPath.c_str(), infoLog);
    }
    return shader;
}

void KeyboardInputHandling(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
    if (action == GLFW_PRESS) {
        g_keymap[key] = true;
    }
    else if (action == GLFW_RELEASE) {
        g_keymap[key] = false;
    }
}

void ProcessInputs(GLFWwindow* window, const float deltaTime) {
    float speed_multiplier = g_keymap[GLFW_KEY_LEFT_SHIFT] ? 5.0f : 1.0f;

    if (g_keymap[GLFW_KEY_W]) editorCamera->MoveForward(speed_multiplier * deltaTime);
    if (g_keymap[GLFW_KEY_S]) editorCamera->MoveBackward(speed_multiplier * deltaTime);
    if (g_keymap[GLFW_KEY_A]) editorCamera->MoveLeft(speed_multiplier * deltaTime);
    if (g_keymap[GLFW_KEY_D]) editorCamera->MoveRight(speed_multiplier * deltaTime);
    if (g_keymap[GLFW_KEY_Q]) editorCamera->MoveDown(speed_multiplier * deltaTime);
    if (g_keymap[GLFW_KEY_E]) editorCamera->MoveUp(speed_multiplier * deltaTime);
}

void MouseInputHandling(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (!g_rotating) return;

    if (g_firstMouse) {
        g_lastX = xpos;
        g_lastY = ypos;
        g_firstMouse = false;
        return;
    }

    const double xoffset = xpos - g_lastX;
    const double yoffset = g_lastY - ypos; // reversed since y-coordinates go

    g_lastX = xpos;
    g_lastY = ypos;

    editorCamera->PivotRotate(glm::vec2(static_cast<float>(xoffset),
        static_cast<float>(yoffset)));
}

void ScrollCallback(GLFWwindow* window, double xoff, double yoff) {
    ImGui_ImplGlfw_ScrollCallback(window, xoff, yoff);
}

void CharCallback(GLFWwindow* window, unsigned int c) {
    ImGui_ImplGlfw_CharCallback(window, c);
}

void MouseButtonInputHandling(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        g_rotating = true;
        g_firstMouse = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        g_rotating = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (action == GLFW_PRESS) {
        g_keymap[button] = true;
    }
    else if (action == GLFW_RELEASE) {
        g_keymap[button] = false;
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(g_width, g_height, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    //Setup platforms
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 400");

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    const GLuint modelVertexShader = GenerateShader("shaders/modelVertex.vs", GL_VERTEX_SHADER);
    const GLuint fragmentShader = GenerateShader("shaders/fragment.fs", GL_FRAGMENT_SHADER);
    const GLuint textureShader = GenerateShader("shaders/texture.fs", GL_FRAGMENT_SHADER);

    int success;
    char infoLog[512];
    const unsigned int modelShaderProgram = glCreateProgram();
    glAttachShader(modelShaderProgram, modelVertexShader);
    glAttachShader(modelShaderProgram, fragmentShader);
    glLinkProgram(modelShaderProgram);
    glGetProgramiv(modelShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(modelShaderProgram, 512, nullptr, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }
    const unsigned int textureShaderProgram = glCreateProgram();
    glAttachShader(textureShaderProgram, modelVertexShader);
    glAttachShader(textureShaderProgram, textureShader);
    glLinkProgram(textureShaderProgram);
    glGetProgramiv(textureShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(textureShaderProgram, 512, nullptr, infoLog);
        printf("Error! Making Shader Program: %s\n", infoLog);
    }

    glDeleteShader(modelVertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(textureShader);

    core::Mesh quad = core::Mesh::GenerateQuad();
    core::Model quadModel({ quad });
    quadModel.Translate(glm::vec3(0, 0, -2.5));
    quadModel.Scale(glm::vec3(5, 5, 1));

    core::Model suzanne = core::AssimpLoader::loadModel("models/nonormalmonkey.obj");
    core::Texture cmgtGatoTexture("textures/CMGaTo_crop.png");

    glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    glClearColor(clearColor.r,
        clearColor.g, clearColor.b, clearColor.a);

    editorCamera = std::make_unique<core::Camera>(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    glfwSetKeyCallback(window, KeyboardInputHandling);
    glfwSetMouseButtonCallback(window, MouseButtonInputHandling);
    glfwSetCursorPosCallback(window, MouseInputHandling);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetCharCallback(window, CharCallback);

    GLint mvpMatrixUniform = glGetUniformLocation(modelShaderProgram, "mvpMatrix");
    GLint textureModelUniform = glGetUniformLocation(textureShaderProgram, "mvpMatrix");
    GLint textureUniform = glGetUniformLocation(textureShaderProgram, "text");

    GLenum drawMode = GL_TRIANGLES;

    double currentTime = glfwGetTime();
    double finishFrameTime = 0.0;
    float deltaTime = 0.0f;
    float rotationStrength = 100.0f;
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwPollEvents();


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::Begin("Raw Engine v2", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::Text("Hello :)");
        if (ImGui::Button("Toggle Wireframe"))
        {
            if (drawMode == GL_TRIANGLES)
                drawMode = GL_LINES;
            else
                drawMode = GL_TRIANGLES;
        }
        ImGui::End();

        glm::mat4 view = editorCamera->GetViewMatrix();
        glm::mat4 projection = editorCamera->GetProjectionMatrix(static_cast<float>(g_width), static_cast<float>(g_height));

        if (!io.WantCaptureKeyboard) {
            ProcessInputs(window, deltaTime);
        }

        ProcessInputs(window, deltaTime);

        suzanne.Rotate(glm::vec3(0.0f, 1.0f, 0.0f), glm::radians(rotationStrength) * static_cast<float>(deltaTime));

        glUseProgram(textureShaderProgram);
        glUniformMatrix4fv(textureModelUniform, 1, GL_FALSE, glm::value_ptr(projection * view * quadModel.GetModelMatrix()));
        glActiveTexture(GL_TEXTURE0);
        glUniform1i(textureUniform, 0);
        glBindTexture(GL_TEXTURE_2D, cmgtGatoTexture.getId());
        quadModel.Render(drawMode);
        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);

        glUseProgram(modelShaderProgram);
        glUniformMatrix4fv(mvpMatrixUniform, 1, GL_FALSE, glm::value_ptr(projection * view * suzanne.GetModelMatrix()));
        suzanne.Render(drawMode);
        glBindVertexArray(0);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        finishFrameTime = glfwGetTime();
        deltaTime = static_cast<float>(finishFrameTime - currentTime);
        currentTime = finishFrameTime;
    }

    glDeleteProgram(modelShaderProgram);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}
