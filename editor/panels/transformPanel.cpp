#include "transformPanel.h"
#include "../../core/objectSystems/gameObject.h"
#include "../../core/objectSystems/components/Transform.h"

namespace editor
{
    void TransformPanel::draw(EditorContext& ctx) {
        ImGui::Begin("Transform", &isVisible);

        if (ctx.currentSelectedGameObject) {
            ImGui::Text("GameObject: %s", ctx.currentSelectedGameObject->GetName().c_str());
            ImGui::Separator();

            auto& transform = ctx.currentSelectedGameObject->transform;

            ImGui::DragFloat3("Position", &transform->position.x, 0.1f);
            ImGui::DragFloat3("Rotation", &transform->rotation.x, 1.0f);
            ImGui::DragFloat3("Scale", &transform->scale.x, 0.01f);
        }
        else {
            ImGui::TextDisabled("No GameObject selected");
        }

        ImGui::End();
    }
}