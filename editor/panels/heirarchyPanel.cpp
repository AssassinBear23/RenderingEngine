#include "hierarchyPanel.h"
#include "../../core/scene.h"
#include "../../core/objectSystems/gameObject.h"

namespace editor
{
    static void DrawGameObjectNode(const std::shared_ptr<core::GameObject>& go, EditorContext& ctx) {
        if (!go) return;

        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
        
        // Highlight if selected
        if (ctx.currentSelectedGameObject == go) {
            flags |= ImGuiTreeNodeFlags_Selected;
        }
        
        // Leaf node if no children
        if (go->GetChildren().empty()) {
            flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        }

        bool nodeOpen = ImGui::TreeNodeEx(go->GetName().c_str(), flags);
        
        // Handle selection
        if (ImGui::IsItemClicked()) {
            ctx.currentSelectedGameObject = go;
        }

        // Draw children if node is open
        if (nodeOpen && !go->GetChildren().empty()) {
            for (const auto& child : go->GetChildren()) {
                DrawGameObjectNode(child, ctx);
            }
            ImGui::TreePop();
        }
    }

    void HierarchyPanel::draw(EditorContext& ctx) {
        ImGui::Begin("Hierarchy", &isVisible);
        
        if (ctx.currentScene) {
            if (ImGui::TreeNodeEx(ctx.currentScene->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                for (const auto& root : ctx.currentScene->Roots()) {
                    DrawGameObjectNode(root, ctx);
                }
                ImGui::TreePop();
            }
        } else {
            ImGui::TextDisabled("No scene loaded");
        }
        
        ImGui::End();
    }
}