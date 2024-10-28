#include "TableStyling.h"
#include "TableDataSelection.h"

namespace Relentless
{
	TableStyling::TableStyling(TableDataSelection* pTableDataSelection) noexcept
		: m_pTableDataSelection{ pTableDataSelection }
	{
	}


	void TableStyling::SetEvenRowColor(const ImVec4& evenRowColor) noexcept
	{
		m_GeneralStyle.EvenRowColor = evenRowColor;
	}

	void TableStyling::SetOddRowColor(const ImVec4& oddRowColor) noexcept
	{
		m_GeneralStyle.OddRowColor = oddRowColor;
	}

	void TableStyling::SetRowHoverColor(const ImVec4& rowHoverColor) noexcept
	{
		m_GeneralStyle.RowHoverColor = rowHoverColor;
	}

	void TableStyling::SetRowSelectedColor(const ImVec4& rowSelectedColor) noexcept
	{
		m_GeneralStyle.RowSelectedColor = rowSelectedColor;
	}

	void TableStyling::SetRowSelectedFocusedColor(const ImVec4& rowSelectedFocusedColor) noexcept
	{
		m_GeneralStyle.RowSelectedFocusedColor = rowSelectedFocusedColor;
	}

	void TableStyling::SetRowAncestorToSelectedColor(const ImVec4& rowAncestorToSelectedColor) noexcept
	{
		m_GeneralStyle.RowAncestorToSelectedColor = rowAncestorToSelectedColor;
	}

	const TableGeneralStyle& TableStyling::GetGeneralStyle() const noexcept
	{
		return m_GeneralStyle;
	}

}