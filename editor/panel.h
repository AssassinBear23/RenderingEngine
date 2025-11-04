#pragma once
#include <string>
#include <imgui.h>

struct EditorContext {
    // Put shared pointers/refs here (scene, camera, selection, io, etc.)
    // e.g. core::Camera* camera = nullptr;
    bool showDemo = false; // optional: global toggle for Dear ImGui demo
};

class Panel {
public:
    explicit Panel(const char* name, bool visible = true)
        : m_name(name), m_visible(visible) {
    }
    virtual ~Panel() = default;

    const char* name() const { return m_name.c_str(); }
    bool* visiblePtr() { return &m_visible; }

    virtual void draw(EditorContext& ctx) = 0;

private:
    std::string m_name;
    bool m_visible;
};
