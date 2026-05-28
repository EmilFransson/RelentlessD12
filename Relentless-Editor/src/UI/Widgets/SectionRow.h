#pragma once
#include "IStylableWidget.h"

namespace Relentless
{
	class SectionRow : public IStylableWidget<SectionRow>
	{
	public:
		SectionRow(StringView aText) noexcept;

		NO_DISCARD float GetSeparatorThickness() const noexcept;

		void OnRender() noexcept override;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		SectionRow* SetSeparatorColor(const Color& aColor) noexcept;
		SectionRow* SetSeparatorThickness(float aThickness) noexcept;
	private:
		String m_Text;
		float m_SeparatorThickness = 1.0f;
	};
}