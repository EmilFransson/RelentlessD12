#include "TableInteraction.h"
#include "Table.h"

namespace Relentless
{
	TableInteraction::TableInteraction(Table* pTable) noexcept
		: m_pTable{ pTable }
	{
	}

	bool TableInteraction::IsDragDropEnabled() const noexcept
	{
		return m_DragDropEnabled;
	}

	void TableInteraction::SetDragDropEnabled(bool enabled) noexcept
	{
		m_DragDropEnabled = enabled;
	}

	void TableInteraction::SetFilter(std::string_view filter) noexcept
	{
		m_Filter = filter;
	}

	bool TableInteraction::UsesFilter() const noexcept
	{
		return !m_Filter.empty();
	}
}

