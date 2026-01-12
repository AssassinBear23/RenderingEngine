#include "inspectorPanel.h"
#include "../../core/ObjectSystems/object.h"
#include "../../core/ObjectSystems/gameObject.h"
#include "../../core/ObjectSystems/componentFactory.h"

namespace editor
{
    static void ShowComponentContextMenu(std::shared_ptr<core::GameObject> selectedObj, std::shared_ptr<core::Component> comp)
    {
        if (ImGui::BeginPopupContextItem())
        {
            if (comp->GetTypeName() != "Transform")
                if (ImGui::MenuItem("Remove Component")) {
                    selectedObj->RemoveComponent(comp);
                    comp->Destroy();
                    ImGui::CloseCurrentPopup();
                }
            if (ImGui::MenuItem("Reset")) { // TODO: implement reset logic per component type (currently just removes and re-adds which doesn't work for transform)
                selectedObj->RemoveComponent(comp);
                auto newComponent = core::ComponentFactory::Create(comp->GetTypeName());
                if (newComponent)
                    selectedObj->AddComponent(newComponent);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    static void ShowAddComponentContextMenu(std::shared_ptr<core::GameObject> selectedObj)
    {
        if (ImGui::BeginPopup("AddComponentPopup"))
        {

            const auto& registeredTypes = core::ComponentFactory::GetRegisteredTypes();

            for (const auto& typeName : registeredTypes)
            {
                if (typeName == "Transform")
                    continue;

                if (ImGui::MenuItem(typeName.c_str()))
                {
                    auto newComponent = core::ComponentFactory::Create(typeName);
                    if (newComponent)
                        selectedObj->AddComponent(newComponent);

                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
    }

    void InspectorPanel::draw(EditorContext& ctx)
    {
        ImGui::Begin(name(), &isVisible);
        const auto& selectedObj = ctx.currentSelectedGameObject;
        if (!selectedObj)
        {
            ImGui::Text("No GameObject selected.");
            ImGui::End();
            return;
        }

        // Use & operator to get pointer to the internal bool value
        ImGui::Checkbox("##enabled", &selectedObj->isEnabled);
        ImGui::SameLine();
        ImGui::SeparatorText(("%s", selectedObj->GetName().c_str()));

        // List components
        const auto& components = selectedObj->GetComponents();
        int componentIndex = 0;
        for (const auto& comp : components)
        {
            ImGui::Spacing();
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 3.0f);
            ImGui::PushID(componentIndex++);
            ImGui::BeginChild("component", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
            // Collapsible header inside the box
            if (comp->GetTypeName() != "Transform")
            {
                ImGui::Checkbox("##comp_enabled", &comp->isEnabled);
                ImGui::SameLine();
            }
            if (ImGui::CollapsingHeader(comp->GetTypeName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::Indent();
                comp->DrawGui();
                ImGui::Unindent();
            }
            ImGui::EndChild();
            ImGui::PopID();
            ImGui::PopStyleVar();

            ShowComponentContextMenu(selectedObj, comp);
        }

        // Add some spacing before the button
        ImGui::Spacing();
        ImGui::Spacing();

        // Center the button with 80% width of available space
        float availWidth = ImGui::GetContentRegionAvail().x;
        float buttonWidth = availWidth * 0.8f;
        float offset = (availWidth - buttonWidth) * 0.5f;
        if (offset > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);


        if (ImGui::Button("Add Component"))
            ImGui::OpenPopup("AddComponentPopup");

        ShowAddComponentContextMenu(selectedObj);

        ImGui::End();
    }
}