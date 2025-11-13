#pragma once

#include <Relentless.h>

namespace Relentless
{
	class TestPanel : public PanelBase
	{
	public:
		TestPanel(const char* aName, ImGuiWindowFlags aFlags);
	protected:
		virtual void OnRender() noexcept override {}
	};
}