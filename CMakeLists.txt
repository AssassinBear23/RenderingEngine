# CMakeList.txt : CMake project for FinalEngine, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_TOOLCHAIN_FILE "C:\\Users\\beerh\\vcpkg\\scripts\\buildsystems\\vcpkg.cmake")

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("FinalEngine")

# Add source to this project's executable.

find_package(nlohmann_json 3.12.0 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

# Locate the 'backends' folder installed by vcpkg (contains the .h/.cpp)
find_path(IMGUI_BACKENDS_DIR
	NAMES imgui_impl_glfw.h imgui_impl_opengl3.h
	PATH_SUFFIXES imgui/backends
)

if (NOT IMGUI_BACKENDS_DIR)
	message(FATAL_ERROR "Could not locate ImGui backends folder (imgui/backends).")
endif()

# Your executable/library
add_executable(FinalEngine
  "main.cpp"
  "core/assimpLoader.cpp"
  "core/model.cpp"
  "core/material.cpp"
  "core/scene.cpp"
  "core/camera.cpp"
  "core/frameBuffer.cpp"
  "core/sceneManager.cpp"
  "core/rendering/mesh.cpp"
  "core/Rendering/shader.h"
  "core/rendering/texture.cpp"
  "core/objectSystems/components/renderer.cpp"
  "core/objectSystems/object.cpp"
  "core/objectSystems/component.cpp"
  "core/objectSystems/gameObject.cpp"
  "core/objectSystems/components/Transform.cpp"
  "core/ObjectSystems/Components/Light.cpp"
  "editor/editor.cpp"
  "editor/inputManager.cpp"
  "editor/panels/viewportPanel.cpp"
  "editor/panels/heirarchyPanel.cpp"
  "editor/panels/inspectorPanel.cpp"
  "editor/panels/transformPanel.cpp"
)

add_custom_command(
		TARGET FinalEngine PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets/shaders/
		${CMAKE_CURRENT_BINARY_DIR}/assets/shaders/)

add_custom_command(
		TARGET FinalEngine PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets/models/
		${CMAKE_CURRENT_BINARY_DIR}/assets/models/)

add_custom_command(
		TARGET FinalEngine PRE_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets/textures/
		${CMAKE_CURRENT_BINARY_DIR}/assets/textures/)

add_custom_target(copy_shaders ALL
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${CMAKE_SOURCE_DIR}/assets/shaders
		${CMAKE_CURRENT_BINARY_DIR}/assets/shaders
		DEPENDS ${SHADER_FILES}
		COMMENT "Copying shaders..."
)

add_dependencies(FinalEngine copy_shaders)

target_include_directories(FinalEngine PRIVATE
	${IMGUI_BACKENDS_DIR}/..        # this is the 'imgui' include dir
	${CMAKE_SOURCE_DIR}				
)

find_package(Stb REQUIRED)
target_include_directories(FinalEngine PRIVATE ${Stb_INCLUDE_DIR})

target_link_libraries(FinalEngine PRIVATE
	nlohmann_json::nlohmann_json
	glad::glad
	glfw
	imgui::imgui
	glm::glm
	assimp::assimp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET FinalEngine PROPERTY CXX_STANDARD 20)
endif()
