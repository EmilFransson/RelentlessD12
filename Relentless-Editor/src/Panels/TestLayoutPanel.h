#pragma once
#include "Panel.h"

namespace Relentless
{
	class TestLayoutPanel : public PanelBase
	{
	public:
		TestLayoutPanel() noexcept;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
	};
}