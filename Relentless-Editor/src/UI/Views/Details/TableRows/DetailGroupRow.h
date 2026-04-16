#pragma once
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	class Button;

	class DetailGroupRow : public ITableRow
	{
	public:
		DetailGroupRow(StringView aName, bool aIsExpanded) noexcept;
		virtual ~DetailGroupRow() noexcept override = default;

		NO_DISCARD Button* GetExpandButton() const noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;
	protected:
		const Color& GetBackgroundColor() const noexcept override;
		uint32 GetNumColumns() noexcept override;

		void OnRenderColumn(uint32 aColumn) noexcept override;
	private:
		std::vector<Ref<IBaseWidget>> m_ColumnWidgets2;
	};
}