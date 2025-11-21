#pragma once
#include "../Panel.h"

namespace editor
{
    class TransformPanel : public Panel {
    public:
        TransformPanel() : Panel("Transform", true) {}

        void draw(EditorContext& ctx) override;
    };
}