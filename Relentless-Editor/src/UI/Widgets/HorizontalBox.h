#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class HorizontalBox : public IWidgetContainer<HorizontalBox>
	{
	public:
		HorizontalBox() noexcept = default;
		virtual ~HorizontalBox() noexcept = default;

		NO_DISCARD Vector2 ReportSize() const noexcept override;
		HorizontalBox* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}