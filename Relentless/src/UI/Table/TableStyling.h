#include "UI/UI.h"

namespace Relentless
{
	class TableData;
	class TableDataSelection;

	struct TableGeneralStyle
	{
		ImVec4 EvenRowColor = ImVec4(21.0f / 255.0f, 21.0f / 255.0f, 21.0f / 255.0f, 255.0f / 255.0f);
		ImVec4 OddRowColor = ImVec4(26.0f / 255.0f, 26.0f / 255.0f, 26.0f / 255.0f, 255.0f / 255.0f);
		ImVec4 RowHoverColor = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
		ImVec4 RowSelectedColor = ImVec4(64.0f / 255.0f, 87.0f / 255.0f, 111.0f / 255.0f, 1.0f);
		ImVec4 RowSelectedFocusedColor = ImVec4(30.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f, 200.0f / 255.0f);
		ImVec4 RowAncestorToSelectedColor = ImVec4(44.0f / 255.0f, 50.0f / 255.0f, 58.0f / 255.0f, 1.0f);
	};

	struct TableRowStyle
	{
		UI::Alignment Alignment = UI::Alignment::Left;
		ImVec4 LabelColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 IconTint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		float IconWeight = 0.6f;
		float Spacing = -1.0f;
	};

	class TableStyling
	{
	public:
		TableStyling(TableDataSelection* pTableDataSelection) noexcept;
		virtual ~TableStyling() noexcept = default;

		void SetEvenRowColor(const ImVec4& evenRowColor) noexcept;
		void SetOddRowColor(const ImVec4& oddRowColor) noexcept;
		void SetRowHoverColor(const ImVec4& rowHoverColor) noexcept;
		void SetRowSelectedColor(const ImVec4& rowSelectedColor) noexcept;
		void SetRowSelectedFocusedColor(const ImVec4& rowSelectedFocusedColor) noexcept;
		void SetRowAncestorToSelectedColor(const ImVec4& rowAncestorToSelectedColor) noexcept;

		[[nodiscard]] const TableGeneralStyle& GetGeneralStyle() const noexcept;
		virtual [[nodiscard]] const TableRowStyle GetRowStyle([[maybe_unused]] const std::shared_ptr<TableData>& pTableData, [[maybe_unused]] uint32_t column) const noexcept { return m_DefaultRowStyle; }
	protected:
		TableDataSelection* m_pTableDataSelection = nullptr;
		TableRowStyle m_DefaultRowStyle;
	private:
		TableGeneralStyle m_GeneralStyle;
	};
}