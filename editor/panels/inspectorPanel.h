#pragma once

#include "../panel.h"

namespace editor
{
	class InspectorPanel : public Panel
	{
	public:
		InspectorPanel() : Panel("Inspector", true) {}
		void draw(EditorContext& ctx) override;
	};
} // namespace editor