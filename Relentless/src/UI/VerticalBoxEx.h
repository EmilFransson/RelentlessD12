#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class VerticalBoxEx : public IWidgetContainer<VerticalBoxEx>
	{
	public:
		VerticalBoxEx(const Vector2 aSize = Vector2::Zero, bool aIsChildRegion = false) noexcept;
		virtual ~VerticalBoxEx() noexcept = default;

		virtual NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; }

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		virtual VerticalBoxEx* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}
