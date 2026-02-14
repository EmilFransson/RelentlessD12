#pragma once
#include "IWidget.h"

namespace Relentless
{
	class Spacer : public IStylableWidget<Spacer>
	{
	public:
		Spacer() noexcept = default;
		virtual ~Spacer() noexcept override = default;

		Vector2 ReportSize() const noexcept override;
	private:
		void OnRender() noexcept override;
		
		NO_DISCARD bool RequiresAssignedSize() const noexcept override;
	};
}