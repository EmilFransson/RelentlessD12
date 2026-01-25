#pragma once

#include <UI/Widgets/Panel.h>

namespace Relentless
{
	class WidgetShowcasePanel : public PanelBase
	{
	public:
		WidgetShowcasePanel() noexcept;
	protected:
		virtual void OnRender() noexcept override {}
	};
}