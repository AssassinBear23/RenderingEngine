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
            
            // Find Transform component
            std::shared_ptr<core::Transform> transform = nullptr;
            for (const auto& comp : ctx.currentSelectedGameObject->GetComponents()) {
                transform = std::dynamic_pointer_cast<core::Transform>(comp);
                if (transform) break;
            }
            
            if (!transform) {
                if (ImGui::Button("Add Transform Component")) {
                    transform = std::make_shared<core::Transform>();
                    ctx.currentSelectedGameObject->AddComponent(transform);
                }
            } else {
                ImGui::DragFloat3("Position", &transform->position.x, 0.1f);
                ImGui::DragFloat3("Rotation", &transform->rotation.x, 1.0f);
                ImGui::DragFloat3("Scale", &transform->scale.x, 0.01f);
            }
        } else {
            ImGui::TextDisabled("No GameObject selected");
        }
        
        ImGui::End();
    }
}