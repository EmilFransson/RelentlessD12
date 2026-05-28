#pragma once

#include "Panel.h"

namespace Relentless
{
	class WidgetShowcasePanel : public PanelBase
	{
	public:
		WidgetShowcasePanel() noexcept;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;

		virtual void OnRender() noexcept override {}
	};
}