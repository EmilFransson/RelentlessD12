#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class HorizontalBox : public IWidgetContainer<HorizontalBox>
	{
	public:
		HorizontalBox(const Vector2 aSize = Vector2::Zero, bool aIsChildRegion = false) noexcept;
		virtual ~HorizontalBox() noexcept = default;

		NO_DISCARD Vector2 ReportSize() const noexcept override;
		
		virtual HorizontalBox* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}
