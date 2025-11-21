#pragma once
#include <string>
#include <memory>
#include <imgui.h>

namespace core
{
    class Scene;
    class GameObject;
}

namespace editor
{
    struct EditorContext {
        // UI Toggles
        bool showDemo = false;

        // Scene Data
        std::shared_ptr<core::Scene> currentScene = nullptr;

        // Selection
        std::shared_ptr<core::GameObject> currentSelectedGameObject = nullptr;
    };

    class Panel {
    public:
        explicit Panel(const char* name, bool visible = true)
            : m_name(name), isVisible(visible) {
        }
        virtual ~Panel() = default;

        const char* name() const { return m_name.c_str(); }

        virtual void draw(EditorContext& ctx) = 0;

        bool isVisible;
    private:
        std::string m_name;
    };
}
