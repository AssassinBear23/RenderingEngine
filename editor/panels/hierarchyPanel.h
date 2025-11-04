#pragma once
#include "../Panel.h"

class HierarchyPanel : public Panel {
public:
    HierarchyPanel() : Panel("Hierarchy", true) {}

    void draw(EditorContext& ctx) override;
};
