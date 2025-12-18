#pragma once

#include "../postProcessingEffectBase.h"

namespace core
{
    namespace postProcessing
    {
        class InvertEffect : public PostProcessingEffectBase
        {
        public:
            InvertEffect(std::weak_ptr<PostProcessingManager> manager);
            
            /// <summary>
            /// 
            /// </summary>
            /// <param name="inputFBO"></param>
            /// <param name="outputFBO"></param>
            /// <param name="width"></param>
            /// <param name="height"></param>
            void Apply(FrameBuffer& inputFBO, FrameBuffer& outputFBO, const int width, const int height) override;
            
            /// <summary>
            /// 
            /// </summary>
            void DrawGui() override;
        };
    } // namespace postProcessing
} // namespace core