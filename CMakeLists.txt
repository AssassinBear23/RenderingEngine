# CMakeList.txt : CMake project for FinalEngine
cmake_minimum_required(VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_TOOLCHAIN_FILE "C:\\Users\\beerh\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
    cmake_policy(SET CMP0141 NEW)
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("FinalEngine")

# ===================================================================
# Find all dependencies at the top level (shared across all targets)
# ===================================================================

find_package(nlohmann_json 3.12.0 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)

# Locate the 'backends' folder installed by vcpkg (contains the .h/.cpp)
find_path(IMGUI_BACKENDS_DIR
    NAMES imgui_impl_glfw.h imgui_impl_opengl3.h
    PATH_SUFFIXES imgui/backends
)

if (NOT IMGUI_BACKENDS_DIR)
    message(FATAL_ERROR "Could not locate ImGui backends folder (imgui/backends).")
endif()

# ===================================================================
# Add subdirectories for modular components
# ===================================================================

add_subdirectory(core)
add_subdirectory(editor)

# ===================================================================
# Main executable
# ===================================================================

add_executable(FinalEngine "main.cpp")

# Link against core and editor libraries
target_link_libraries(FinalEngine PRIVATE 
    CoreEngine
    EditorLib
)

# ===================================================================
# Asset copying function (reusable for multiple targets)
# ===================================================================

function(copy_assets_to_target target)
    # Copy shaders
    add_custom_command(
        TARGET ${target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/shaders/
        $<TARGET_FILE_DIR:${target}>/assets/shaders/
        COMMENT "Copying shaders for ${target}..."
    )
    
    # Copy models
    add_custom_command(
        TARGET ${target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/models/
        $<TARGET_FILE_DIR:${target}>/assets/models/
        COMMENT "Copying models for ${target}..."
    )
    
    # Copy textures
    add_custom_command(
        TARGET ${target} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets/textures/
        $<TARGET_FILE_DIR:${target}>/assets/textures/
        COMMENT "Copying textures for ${target}..."
    )
endfunction()

# Copy assets for the main executable
copy_assets_to_target(FinalEngine)

# ===================================================================
# Optional: Set FinalEngine as the startup project in Visual Studio
# ===================================================================

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT FinalEngine)
