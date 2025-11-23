#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include "core/rendering/mesh.h"
#include "core/rendering/texture.h"
#include "core/material.h"
#include "core/assimpLoader.h"
#include "core/camera.h"
#include "core/scene.h"
#include "core/objectSystems/gameObject.h"
#include "core/objectSystems/components/transform.h"
#include "core/objectSystems/components/renderer.h"

#include "editor/Editor.h"

#define VSTUDIO

#ifdef VSTUDIO
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#endif
#include <editor/panels/viewportPanel.h>
#include <editor/panels/hierarchyPanel.h>
#include <editor/panels/transformPanel.h>
using namespace editor;

int g_width = 800;
int g_height = 600;
std::unordered_map<int, bool> g_keymap;
std::unique_ptr<core::Camera> editorCamera;

static bool g_rotating = false;
static bool g_firstMouse = true;
static double g_lastX = 0.0;
static double g_lastY = 0.0;


void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
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

// Helper function to recursively render all GameObjects in the scene
void RenderGameObject(const std::shared_ptr<core::GameObject>& go,
                      const glm::mat4& parentMatrix,
                      const glm::mat4& view,
                      const glm::mat4& projection) {
    if (!go) return;

    // Get transform component
    std::shared_ptr<core::Transform> transform = nullptr;
    for (const auto& comp : go->GetComponents()) {
        transform = std::dynamic_pointer_cast<core::Transform>(comp);
        if (transform) break;
    }

    // Calculate world matrix
    glm::mat4 localMatrix = transform ? transform->GetLocalMatrix() : glm::mat4(1.0f);
    glm::mat4 worldMatrix = parentMatrix * localMatrix;

    // Render if this GameObject has a Renderer
    std::shared_ptr<core::Renderer> renderer = nullptr;
    for (const auto& comp : go->GetComponents()) {
        renderer = std::dynamic_pointer_cast<core::Renderer>(comp);
        if (renderer) break;
    }

    if (renderer && renderer->GetMaterial()) {
        // Set MVP matrix in material
        glm::mat4 mvp = projection * view * worldMatrix;
        renderer->GetMaterial()->SetMat4("mvpMatrix", mvp);

        // Render with the material
        renderer->Render(GL_TRIANGLES);
    }

    // Recursively render children
    for (const auto& child : go->GetChildren()) {
        RenderGameObject(child, worldMatrix, view, projection);
    }
}

int main() {
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(g_width, g_height, "FinalEngine", nullptr, nullptr);
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

	Editor editor;
	editor.init(window, "#version 400");

	auto& viewport = editor.addPanel<ViewportPanel>(editor);
	editor.addPanel<HierarchyPanel>();
	editor.addPanel<TransformPanel>();

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
	if (!success) {
		glGetProgramInfoLog(modelShaderProgram, 512, nullptr, infoLog);
		printf("Error! Making Shader Program: %s\n", infoLog);
	}

	// Texture shader program (for Quad)
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

	// Create Scene
	auto scene = std::make_shared<core::Scene>("Main Scene");

	// Create Suzanne GameObject
	auto suzanneGO = scene->CreateObject("Suzanne");

	// Load Suzanne model and create material
	core::Model suzanneModel = core::AssimpLoader::loadModel("assets/models/nonormalmonkey.obj");
	auto suzanneMaterial = std::make_shared<core::Material>(modelShaderProgram);

	// Create renderer with meshes from model
	std::vector<core::Mesh> suzanneMeshes;
	// Note: We need to extract meshes from the Model class
	// For now, assuming Model has a way to get meshes or we refactor it
	auto suzanneRenderer = std::make_shared<core::Renderer>(suzanneModel.GetMeshes(), suzanneMaterial);
	suzanneGO->AddComponent(suzanneRenderer);

	// Create Quad GameObject
	auto quadGO = scene->CreateObject("Quad");
	quadGO->transform->position = glm::vec3(0, 0, -2.5f);
	quadGO->transform->scale = glm::vec3(5, 5, 1);

	// Create quad mesh and material with texture
	core::Mesh quadMesh = core::Mesh::GenerateQuad();
	auto quadTexture = std::make_shared<core::Texture>("assets/textures/CMGaTo_crop.png");
	auto quadMaterial = std::make_shared<core::Material>(textureShaderProgram);
	quadMaterial->SetTexture("text", quadTexture, 0);

	auto quadRenderer = std::make_shared<core::Renderer>(quadMesh, quadMaterial);
	quadGO->AddComponent(quadRenderer);

	// Set scene in editor context
	Editor::editorCtx.currentScene = scene;

	glm::vec4 clearColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);

	editorCamera = std::make_unique<core::Camera>(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	glfwSetKeyCallback(window, KeyboardInputHandling);
	glfwSetMouseButtonCallback(window, MouseButtonInputHandling);
	glfwSetCursorPosCallback(window, MouseInputHandling);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwSetCharCallback(window, CharCallback);

	double currentTime = glfwGetTime();
	double finishFrameTime = 0.0;
	float deltaTime = 0.0f;

	ImGuiIO& io = ImGui::GetIO();

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		editor.beginFrame();
		editor.draw();

		GLuint fb = editor.framebuffer();
		int vw = editor.getViewportWidth();
		int vh = editor.getViewportHeight();
		if (vw <= 0 || vh <= 0 || editor.framebuffer() == 0) {
			// Skip 3D rendering this frame
		}
		else {
			glBindFramebuffer(GL_FRAMEBUFFER, editor.framebuffer());
			glViewport(0, 0, editor.getViewportWidth(), editor.getViewportHeight());
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// Render 3D scene
			glm::mat4 view = editorCamera->GetViewMatrix();
			glm::mat4 projection = editorCamera->GetProjectionMatrix(
				static_cast<float>(editor.getViewportWidth()),
				static_cast<float>(editor.getViewportHeight())
			);

			if (editor.viewportFocused()) {
				if (!io.WantCaptureMouse && !io.WantCaptureKeyboard) {
					ProcessInputs(window, deltaTime);
				}
			}

			// Rotate Suzanne
			float rotationSpeed = editor.rotationSpeedDegSec;
			suzanneGO->transform->rotation.y += rotationSpeed * deltaTime;

			// Render all GameObjects in the scene
			for (const auto& rootGO : scene->Roots()) {
				RenderGameObject(rootGO, glm::mat4(1.0f), view, projection);
			}	

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
