#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class HorizontalBoxEx : public IWidgetContainer<HorizontalBoxEx>
	{
	public:
		HorizontalBoxEx(const Vector2 aSize = Vector2::Zero, bool aIsChildRegion = false) noexcept;
		virtual ~HorizontalBoxEx() noexcept = default;

		virtual NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; }

		NO_DISCARD Vector2 ReportSize() const noexcept override;
		
		virtual HorizontalBoxEx* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}
