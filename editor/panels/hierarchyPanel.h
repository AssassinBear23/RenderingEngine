#pragma once
#include "../Panel.h"
#include "../../Core/scene.h"
#include "../../Core/ObjectSystems/GameObject.h"

namespace editor
{
    class HierarchyPanel : public Panel {
    public:
        HierarchyPanel() : Panel("Hierarchy", true) {}

        void draw(EditorContext& ctx) override;
    };
}