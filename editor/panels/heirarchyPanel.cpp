#include "hierarchyPanel.h"

void HierarchyPanel::draw(EditorContext& ctx) {
    ImGui::Begin("Hierarchy", visiblePtr());
    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::BulletText("Camera");
        if (ImGui::TreeNode("Suzanne")) {
            ImGui::BulletText("MeshRenderer");
            ImGui::BulletText("Transform");
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Quad")) {
            ImGui::BulletText("MeshRenderer");
            ImGui::BulletText("Transform");
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
    ImGui::End();
}
