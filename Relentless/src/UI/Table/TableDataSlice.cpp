#include "TableDataSlice.h"
#include "Table.h"
#include "TableData.h"

namespace Relentless
{
	TableDataSlice::TableDataSlice(Table* table, TableData* pOwner) noexcept
		: m_pTable{table}, m_pOwningData{ pOwner }
	{

	}

	void TableDataSlice::Add(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		m_SliceDatas.push_back(pTableData);
		OnEntryAdded(pTableData);
	}

	const std::vector<std::shared_ptr<TableData>>& TableDataSlice::GetData() const
	{
		return m_SliceDatas;
	}

	const TableData* TableDataSlice::GetOwner() const noexcept
	{
		return m_pOwningData;
	}

	Table* TableDataSlice::GetTable() const noexcept
	{
		return m_pTable;
	}

}