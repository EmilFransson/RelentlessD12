#pragma once
#include "IWidget.h"

namespace Relentless
{
	class Separator : public IStylableWidget<Separator>
	{
	public:
		Separator() noexcept = default;
		Separator(const String& label, float spacing = 4.0f) noexcept;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;
		void OnRender() noexcept override;
	private:
		String m_Label;
		float m_Spacing = 0.0f;
	};
}