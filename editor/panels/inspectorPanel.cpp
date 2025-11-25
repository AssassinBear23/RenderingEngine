#include "inspectorPanel.h"
#include "../../core/ObjectSystems/object.h"
#include "../../core/ObjectSystems/gameObject.h"

namespace editor
{
	void InspectorPanel::draw(EditorContext& ctx)
	{
		ImGui::Begin(name(), &isVisible);
		const auto& selectedObj = ctx.currentSelectedGameObject;
		if (selectedObj)
		{
			ImGui::Text("Selected GameObject: %s", selectedObj->GetName().c_str());

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
				if (ImGui::CollapsingHeader(comp->GetTypeName().c_str(), ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::Indent();
					comp->DrawGui();
					ImGui::Unindent();
				}
				ImGui::EndChild();
				ImGui::PopID();
				ImGui::PopStyleVar();
			}
		}
		else
		{
			ImGui::Text("No GameObject selected.");
		}
		ImGui::End();
	}
}