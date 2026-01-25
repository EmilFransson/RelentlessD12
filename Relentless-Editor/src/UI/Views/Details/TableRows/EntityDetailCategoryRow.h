#pragma once
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	class Button;

	class EntityDetailCategoryRow : public ITableRow
	{
	public:
		EntityDetailCategoryRow(std::string_view aName, bool aIsExpanded) noexcept;
		virtual ~EntityDetailCategoryRow() noexcept override = default;

		NO_DISCARD Button* GetExpandButton() const noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

	protected:
		NO_DISCARD float CalcDesiredWidth() const noexcept override;
		const Color& GetBackgroundColor() const noexcept override;
		uint32 GetNumColumns() noexcept override;
		bool IsDragDropEligible() noexcept override;
		void OnRenderColumn(uint32 aColumn) noexcept override;
	};	
}