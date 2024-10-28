#include "TableData.h"
#include "TableDataSlice.h"

namespace Relentless
{
	TableData::TableData(Table* pTable) noexcept
		: m_pTable{ pTable }
	{
		m_pSlice = std::make_unique<TableDataSlice>(m_pTable, this);
	}

	void TableData::AddChild(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		m_pSlice->Add(pTableData);
	}

	bool TableData::HasChildren() const noexcept
	{
		return m_pSlice && m_pSlice->GetData().size() > 0;
	}

	void TableData::SetTableDataSlice(std::unique_ptr<TableDataSlice>&& pSlice) noexcept
	{
		m_pSlice = std::move(pSlice);
	}

}