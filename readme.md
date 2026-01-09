# FinalEngine - Modern C++20 Rendering Engine

A feature-rich, OpenGL-based 3D rendering engine built with C++20, designed for real-time graphics rendering and scene management. This project showcases modern graphics programming techniques including deferred rendering, post-processing effects, and an integrated editor interface.

![FinalEngine Screenshot](docs/screenshot.png)<br>
^Replace with screenshot of actual engine^

---
## 📖 Project Overview
This project started as a hand-in for a course assignement at my study,
but due to me really falling in love with working on this project it has evolved into a hobby project and my main porfolio item.
The overall goal of the course was to learn C++ aswell as work with OpenGL and C++ libraries.

**Primary Goals (from the study):**
- Demonstrate capable handling of C++ and OpenGL features.
- Provide real-time rendering of a 3D scene with lighting, textures and different objects.
- Showcase an implementation of post-processing effects.

**Primary Goals (personal):**
- Create and maintain a clean, modular codebase using modern C++20 features.
- Implement a component-based architecture inspired by Unity.
---

## Features

### Current Features

#### Rendering
- **OpenGL 4.3 Core Profile** with debug callback support
- **Multiple Render Targets (MRT)** for advanced rendering techniques
- **Shadow Mapping** with configurable light types (Directional, Point, Spot)
- **Normal Mapping** for enhanced surface detail
- **Post-Processing Pipeline** with stackable effects:
  - Bloom effect with adjustable threshold
  - Color inversion effect
  - Custom effect support through base class
- **Framebuffer System** with dynamic resizing
- **Custom Shader System** with `#include` directive support for modular shader code

#### Architecture
- **Component-Based Object System** inspired by Unity
  - GameObject hierarchy with parent-child relationships
  - Transform component (position, rotation, scale)
  - Renderer component for mesh rendering
  - Light component (directional, point, spot lights)
- **Scene Management** with multiple scene support
  - Currently hardcoded scenes only.
- **Material System** with texture and uniform management
  - Also hardcoded, no material editor yet.
- **Model Loading** via Assimp (FBX, OBJ, and more)

#### Editor
- **ImGui-Based Interface** with dockable panels
- **Viewport Panel** with scene visualization
- **Hierarchy Panel** for scene heirarchy navigation
- **Inspector Panel** for component editing
- **Post-Processing Panel** for effect configuration
- **FPS Camera** with WASD movement and mouse look

#### Asset Management
- Texture loading with STB Image (JPG, PNG support)
- Model loading with Assimp

### 🎯 Roadmap
Future enhancements and features in order of priority:

- **Fog** for atmospheric effect
- **Scene Serialization** (save/load)
- **HDR Rendering** with tone mapping
- **Shader Hot-Reloading** for rapid shader development
- **More Post-Processing Effects** (Motion Blur, Depth of Field, Fog)
- **Skybox Support** for environment backgrounds
- **Physically-Based Rendering (PBR)** material system
- **Runtime Script System** (potential Lua/C# integration)
<br><br>
## ⚒️ Technologies Used

### Core Technologies

| Technology | Version | Purpose |
|------------|---------|---------|
| **C++** | C++20 | Modern language features (concepts, modules support, ranges) |
| **CMake** | 3.8+ | Cross-platform build system with vcpkg integration |
| **OpenGL** | 4.3+ | Core graphics API for rendering |
| **GLFW** | 3.x | Window management and input handling |
| **GLAD** | - | OpenGL function loader |

### Libraries & Dependencies

| Library | Purpose | Why Chosen |
|---------|---------|------------|
| **GLM** | Mathematics library | Industry-standard, header-only library for graphics math. Provides vec, mat types compatible with GLSL |
| **Assimp** | 3D model loading | Supports 40+ file formats (FBX, OBJ, GLTF). Handles complex model hierarchies and animations |
| **STB Image** | Image loading | Single-header library, simple integration. Supports major formats (PNG, JPG, BMP) |
| **ImGui** | Immediate-mode GUI | Lightweight, easy integration. Perfect for editor interfaces and debug tools |
| **nlohmann/json** | JSON parsing | Modern C++ API, header-only. For future scene serialization |
| **vcpkg** | Package management | Simplifies dependency management across platforms |

### Why These Technologies?

**OpenGL 4.3+**: I started with this technology due to my studies, as it's the required graphics API for the course.
I have so far found it nice to work with due to high amount of documentation available, as well as the fact it supports all platforms.

**C++20**: Leverages modern C++ features like concepts, templates, and `enable_shared_from_this`.
Smart pointers (`shared_ptr`, `weak_ptr`) manage object lifetimes safely.

**ImGui**: Allows rapid development of editor UI without complexity of a full GUI framework.
Its immediate-mode design fits well with the current development stage of this project.

**CMake + vcpkg**: Ensures reproducible builds across Windows, Linux, and macOS.
vcpkg handles all dependencies automatically, reducing setup friction.
<br><br>
## 📋 Prerequisites
- **C++20 Compatible Compiler**:
  - MSVC 2019 16.11+ (Visual Studio 2019 or 2022)
  - GCC 10+
  - Clang 11+
- **CMake** 3.8 or higher
- **vcpkg** (for dependency management)
- **OpenGL 4.3+** capable GPU and drivers
- **Git** for cloning the repository
<br><br>
## 🎮 Controls

| Input | Action |
|-------|--------|
| **W/A/S/D** | Move camera forward/left/backward/right |
| **Q/E** | Move camera down/up |
| **Right Mouse + Drag** | Rotate camera |
| **Scroll Wheel** | Adjust camera movement speed |
| **F** | Focus selected object (planned) |
<br><br>
## Getting Started

### 1. Install vcpkg

If you haven't already, install vcpkg:

### 2. Clone the Repository

Clone the repository using your preferred method.

### 3. Install Dependencies
Use vcpkg to install the required dependencies using one of the following methods depending on your IDE:
You can run the following command in your terminal if you are using VS:
```bash
vcpkg install glfw3 glad glm assimp stb imgui[docking-experimental,glfw-binding,opengl3-binding] nlohmann-json
```
If you are using VSCode or CLion, you can use the VCPKG GUI to install the dependencies.

### 3. Configure CMake Toolchain

Edit `CMakeLists.txt` and update the vcpkg toolchain path to match your installation.

e.g: set(CMAKE_TOOLCHAIN_FILE "C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake")

### 4. Build the Project
Using VisualStudio:
- Open the project folder in Visual Studio.
- Select `CMakeLists.txt` as the active file, and then save.
  - This will trigger CMake to configure and generate the build files.
- At the top menu, select `Build > Build All` to compile the project.
- Once built, you can run the project by selecting `Debug > Start Debugging` or pressing `F5`.
<br><br>
There is currently no _real_ use in running the .exe file as of now, as there is no project explorer, or scene saving/loading implemented.
<br><br>
## 🏗️ Architecture & Design Decisions
### Component-Based Object System

The engine uses a component-based architecture where GameObjects are containers for components (Transform, Renderer, Light, etc.). This design:

- **Separates concerns**: Each component handles specific functionality
- **Promotes reusability**: Components can be attached to any GameObject
- **Simplifies extension**: New component types can be added without modifying existing code
- **Enables data-driven design**: Components can be serialized/deserialized (future feature)

**Key Classes:**
- `GameObject`: Container with transform, parent-child hierarchy
- `Component`: Base class for all components
- `Transform`: Position, rotation, scale
- `Renderer`: Mesh rendering with material
- `Light`: Lighting calculations (directional, point, spot)
<br><br>

### Material System
The Material system abstracts shader uniforms and textures:
This design keeps rendering code clean and makes it easy to swap shaders without changing GameObject code.
<br><br>
### Multiple Render Targets (MRT)

The engine uses MRT to output both scene color and bright pixels in a single pass:<br>
```glsl
layout (location = 0) out vec4 FragColor;       // Main scene
layout (location = 1)out vec4 BrightColor;      // Bloom extraction
```
This enables efficient post-processing effects like bloom without additional scene passes.
<br><br>
### Shadow Mapping Pipeline

Shadows are rendered in two passes:

1. **Shadow Pass**: Render scene depth from light's perspective to depth texture
2. **Lighting Pass**: Sample shadow map to determine if fragments are in shadow

The `Scene` class manages shadow map framebuffers and coordinates both passes automatically.
<br><br>
### Post-Processing Stack

Post-processing effects are composable and stackable:<br>
```c++
postProcessingManager->RegisterEffect<BloomEffect>(material); 
postProcessingManager->EnableEffect("Bloom");
```
Each effect inherits from `PostProcessingEffectBase` and implements `Apply()`. Effects are applied sequentially using ping-pong framebuffers.
<br><br>
### Why OpenGL Over Vulkan/DirectX 12?

While modern APIs like Vulkan offer better performance and control, OpenGL 4.3 was chosen for:

- **Requirement**: Mandated by course assignment
- **Learning focus**: Abstracts low-level details, allowing focus on rendering techniques and engine architecture
- **Rapid prototyping**: Less boilerplate code compared to Vulkan
- **Cross-platform**: Single codebase for Windows, Linux, macOS
<br><br>

## Acknowledgments

- **LearnOpenGL** - Excellent resource for modern OpenGL techniques.
- **ImGui** - For the fantastic immediate-mode GUI library and extensive demo.
- **[Lucas van Dam](https://github.com/Lucas-van-Dam)** - For help and guidance during the course assignment.
- **Open Source Libraries** - Thanks to the developers of GLFW, GLM, Assimp, STB Image, and nlohmann/json for their invaluable tools.

## Contact

**AssassinBear23**  
[![GitHub](https://img.shields.io/badge/GitHub-181717?style=for-the-badge&logo=github&logoColor=white)](https://github.com/AssassinBear23)
[![Email](https://img.shields.io/badge/Email-D14836?style=for-the-badge&logo=gmail&logoColor=white)](mailto:beer.huizer@gmail.com)
