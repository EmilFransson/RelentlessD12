#pragma once
#include <UI/Widgets/Panel.h>

namespace Relentless
{
	class EnvironmentEditorPanel : public PanelBase
	{
	public:
		EnvironmentEditorPanel(const std::vector<Ref<Environment>>& someEnvironments) noexcept;
	};
}