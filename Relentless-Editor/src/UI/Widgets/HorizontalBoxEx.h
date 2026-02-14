#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class HorizontalBoxEx : public IWidgetContainer<HorizontalBoxEx>
	{
	public:
		HorizontalBoxEx() noexcept = default;
		virtual ~HorizontalBoxEx() noexcept = default;

		NO_DISCARD Vector2 ReportSize() const noexcept override;
		HorizontalBoxEx* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}