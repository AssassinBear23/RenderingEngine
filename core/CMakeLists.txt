# Core rendering engine library - platform-independent, no editor dependencies

add_library(CoreEngine STATIC
    # Core systems
    assimpLoader.cpp
    model.cpp
    material.cpp
    scene.cpp
    camera.cpp
    sceneManager.cpp
    
    # Rendering
    rendering/mesh.cpp
    rendering/shader.h
    rendering/texture.cpp
    rendering/frameBuffer.cpp
    
    # Post-processing
    rendering/postProcessing/postProcessingManager.cpp
    rendering/postProcessing/postProcessingEffectBase.cpp
    rendering/postProcessing/effects/postProcessingEffects.h
    rendering/postProcessing/effects/bloomEffect.cpp
    rendering/postProcessing/effects/invertEffect.cpp
    rendering/postProcessing/effects/fogEffect.cpp
    
    # Object systems
    objectSystems/components/renderer.cpp
    objectSystems/object.cpp
    objectSystems/component.cpp
    objectSystems/gameObject.cpp
    objectSystems/components/Transform.cpp
    objectSystems/components/Light.cpp
)

# Public headers - accessible to all targets linking against CoreEngine
target_include_directories(CoreEngine PUBLIC
    ${CMAKE_SOURCE_DIR}         # Root directory for #include <core/...>
    ${Stb_INCLUDE_DIR}          # STB image header
)

# Link core dependencies
target_link_libraries(CoreEngine PUBLIC
    nlohmann_json::nlohmann_json
    glad::glad
    glfw                        # For window context (minimal usage in core)
    glm::glm
    assimp::assimp
)

# Set C++20 standard
set_property(TARGET CoreEngine PROPERTY CXX_STANDARD 20)
set_property(TARGET CoreEngine PROPERTY CXX_STANDARD_REQUIRED ON)

# Optional: Add a folder for Visual Studio solution
set_property(TARGET CoreEngine PROPERTY FOLDER "Engine")