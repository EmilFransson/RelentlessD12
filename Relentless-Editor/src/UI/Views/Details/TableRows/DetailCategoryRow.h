#pragma once
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	class Button;

	class DetailCategoryRow : public ITableRow
	{
	public:
		DetailCategoryRow(std::string_view aName, bool aIsExpanded) noexcept;
		virtual ~DetailCategoryRow() noexcept override = default;

		NO_DISCARD Button* GetExpandButton() const noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

	protected:
		const Color& GetBackgroundColor() const noexcept override;
		uint32 GetNumColumns() noexcept override;

		bool IsDragDropEligible() noexcept override;

		void OnRenderColumn(uint32 aColumn) noexcept override;
	private:
		std::vector<Ref<IBaseWidget>> m_ColumnWidgets2;
	};
}