#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class VerticalBoxEx : public IWidgetContainer<VerticalBoxEx>
	{
	public:
		VerticalBoxEx() noexcept = default;
		virtual ~VerticalBoxEx() noexcept = default;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		VerticalBoxEx* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}