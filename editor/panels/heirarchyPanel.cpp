#include "hierarchyPanel.h"
#include "../../Core/sceneManager.h"
#include "../../core/scene.h"
#include "../../core/objectSystems/gameObject.h"
#include "../../core/objectSystems/componentFactory.h"

namespace editor
{
    static void ShowGameObjectContextMenu(core::GameObject* go)
    {
        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Add Child")) {
                auto newChild = core::GameObject::Create("New Child");
                newChild->SetParent(std::static_pointer_cast<core::GameObject>(go->shared_from_this()));
                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Duplicate")) { 
                auto newDuplicate = core::GameObject::Create(go->GetName() + " Copy");
                if (auto parent = go->GetParent().lock())
                    newDuplicate->SetParent(parent);
                else
                    newDuplicate->SetParent(nullptr);
                
                // TODO: Deep copy components and children.

                ImGui::CloseCurrentPopup();
            }
            if (ImGui::MenuItem("Delete", "Del")) { 
                go->Destroy();
            }
            ImGui::EndPopup();
        }
    }

    static void ShowEmptySpaceContextMenu(EditorContext& ctx)
    {
        // Open popup when right-clicking on empty space in the window
        if (ImGui::BeginPopupContextWindow("HierarchyEmptySpaceMenu", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
        {
            auto currentScene = ctx.sceneManager->GetCurrentScene();

            if (currentScene)
            {
                if (ImGui::MenuItem("Create Empty GameObject"))
                {
                    auto newObject = core::GameObject::Create("New GameObject");
                    currentScene->AddRootGameObject(newObject);
                    ctx.currentSelectedGameObject = newObject;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::Separator();

                // Optional: Add Categorized Submenus
                if (ImGui::BeginMenu("Lighting"))
                {
                    if (ImGui::MenuItem("Light"))
                    {
                        auto newObject = core::GameObject::Create("Light");
                        
                        auto light = core::ComponentFactory::Create("Light");
                        if (light) newObject->AddComponent(light); // When gizmos are implemented, the light gizmo can show the lights position (currently it will be invisible).

                        currentScene->AddRootGameObject(newObject);
                        ctx.currentSelectedGameObject = newObject;
                    }

                    ImGui::EndMenu();
                }
            }
            else
            {
                ImGui::TextDisabled("No scene loaded");
            }

            ImGui::EndPopup();
        }
    }

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

        ShowGameObjectContextMenu(go.get());

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

        auto currentScene = ctx.sceneManager->GetCurrentScene();

        ShowEmptySpaceContextMenu(ctx);

        if (currentScene) {
            if (ImGui::TreeNodeEx(currentScene->GetName().c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
                for (const auto& root : currentScene->Roots()) {
                    DrawGameObjectNode(root, ctx);
                }
                ImGui::TreePop();
            }
        }
        else {
            ImGui::TextDisabled("No scene loaded");
        }

        ImGui::End();
    }

}