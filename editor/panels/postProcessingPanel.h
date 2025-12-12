#pragma once

#include "../panel.h"
#include <memory>
#include <vector>

namespace core
{
    namespace postProcessing
    {
        class PostProcessingEffectBase;
        class PostProcessingManager;
    }
}

namespace editor
{
    class PostProcessingPanel : public Panel
    {
    public:
        PostProcessingPanel(core::postProcessing::PostProcessingManager* manager);
        ~PostProcessingPanel() = default;

        void draw(EditorContext& ctx) override;

    private:
        core::postProcessing::PostProcessingManager* m_postProcessingManager;
    };
} // namespace editor