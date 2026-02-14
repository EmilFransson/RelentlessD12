#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class VerticalBox : public IWidgetContainer<VerticalBox>
	{
	public:
		VerticalBox(const Vector2 aSize = Vector2::Zero, bool aIsChildRegion = false) noexcept;
		virtual ~VerticalBox() noexcept = default;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		virtual VerticalBox* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}
