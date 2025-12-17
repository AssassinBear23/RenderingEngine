#include "postProcessingPanel.h"
#include <core/rendering/postProcessing/postProcessingManager.h>
#include <core/rendering/postProcessing/postProcessingEffectBase.h>
#include <imgui.h>

namespace editor
{
    PostProcessingPanel::PostProcessingPanel(core::postProcessing::PostProcessingManager* manager)
        : Panel("Post Processing", true)
        , m_postProcessingManager(manager)
    {
    }

    void PostProcessingPanel::draw(EditorContext& ctx)
    {
        if (!ImGui::Begin(name(), &isVisible))
        {
            ImGui::End();
            return;
        }

        if (!m_postProcessingManager)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "No Post Processing Manager available!");
            ImGui::End();
            return;
        }

        ImGui::Text("Post Processing Stack");
        ImGui::Separator();

        auto effects = m_postProcessingManager->GetEffects();

        if (effects.empty())
        {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No effects in the stack.");
        }
        else
        {
            for (auto& effect : effects)
            {
                ImGui::PushID(effect.get());

                ImGui::Checkbox(effect->GetName().c_str(), &(effect->isEnabled));

                ImGui::Indent();
                effect->DrawGui();
                ImGui::Unindent();

                ImGui::Separator();
                ImGui::PopID();
            }
        }

        ImGui::End();
    }
} // namespace editor