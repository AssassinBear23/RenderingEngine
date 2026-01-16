# Editor library - ImGui-based interface, depends on CoreEngine

add_library(EditorLib STATIC
    editor.cpp
    inputManager.cpp
    
    # Editor panels
    panels/viewportPanel.cpp
    panels/heirarchyPanel.cpp
    panels/inspectorPanel.cpp
    panels/postProcessingPanel.cpp
)

# Public headers for editor
target_include_directories(EditorLib PUBLIC
    ${CMAKE_SOURCE_DIR}                 # Root directory for #include <editor/...>
    ${IMGUI_BACKENDS_DIR}/..            # ImGui include directory
)

# Link against core engine and ImGui
target_link_libraries(EditorLib PUBLIC
    CoreEngine                          # Public: exposes core to targets linking EditorLib
    imgui::imgui                        # ImGui for UI
)

# Set C++20 standard
set_property(TARGET EditorLib PROPERTY CXX_STANDARD 20)
set_property(TARGET EditorLib PROPERTY CXX_STANDARD_REQUIRED ON)

# Optional: Add a folder for Visual Studio solution
set_property(TARGET EditorLib PROPERTY FOLDER "Editor")