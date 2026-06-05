#pragma once
#include "IStylableWidget.h"

namespace Relentless
{
	class Separator : public IStylableWidget<Separator>
	{
	public:
		Separator(bool aIsHorizontal = true) noexcept;

		NO_DISCARD float GetThickness() const noexcept;

		void OnRender() noexcept override;
		
		NO_DISCARD Vector2 ReportSize() const noexcept override;
		
		virtual Separator* SetActiveColor(const Color& aColor) noexcept override;
		Separator* SetThickness(float aThickness) noexcept;
	protected:
		NO_DISCARD bool RequiresAssignedSize() const noexcept override;

		virtual bool TryGetHoverRect(ImRect& aRect) const noexcept override;
	private:
		ImRect m_Rect;
		float m_Thickness = 1.0f;
		bool m_IsHorizontal = true;
	};
}