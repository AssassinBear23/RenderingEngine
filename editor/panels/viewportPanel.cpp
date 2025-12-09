#include "ViewportPanel.h"
#include <iostream>

namespace editor
{
    ViewportPanel::ViewportPanel(Editor& editor)
        : Panel("Viewport", true)
    {
        // let the editor know “I’m the viewport”
        editor.m_viewport = this;
    }

    void ViewportPanel::draw(EditorContext& ctx)
    {
        if (!isVisible) return;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport", &isVisible);
        ImGui::PopStyleVar();

        ImVec2 avail = ImGui::GetContentRegionAvail();
        int w = (int)avail.x;
        int h = (int)avail.y;

        if(w < 0 || h < 0)
        {
            ImGui::End();
            return;
        }

        m_frameBuffer.Resize(w, h);

        m_focused = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows);

        // Draw the color attachment (flip v)
        if (m_frameBuffer.IsValid())
        {
            ImGui::Image(
                (ImTextureID)(intptr_t)m_frameBuffer.GetColorAttachment(),
                avail,
                ImVec2(0, 1),
                ImVec2(1, 0)
            );
        }

        ImGui::End();
    }
}