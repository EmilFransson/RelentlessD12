#pragma once
#include "IWidgetContainer.h"

namespace Relentless
{
	class VerticalBox : public IWidgetContainer<VerticalBox>
	{
	public:
		VerticalBox() noexcept = default;
		virtual ~VerticalBox() noexcept = default;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		VerticalBox* SetSpacing(float aSpacing) noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	};
}