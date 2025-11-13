#pragma once
#include <Relentless.h>

namespace Relentless
{
	class EntityDetailRow : public ITableRow
	{
	protected:
		NO_DISCARD float CalcDesiredWidth() const noexcept override;

		const Color& GetBackgroundColor() const noexcept override;
		uint32 GetNumColumns() noexcept override;

		bool IsDragDropEligible() noexcept override;

		void OnRenderColumn(uint32 aColumn) noexcept override;
	public:
		NO_DISCARD Vector2 ReportSize() const noexcept override;

	};
}