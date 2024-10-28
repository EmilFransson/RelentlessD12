#include "OutlinerTableDataSelection.h";
#include "OutlinerTable.h"
#include "OutlinerTableData.h"

namespace Relentless
{
	OutlinerTableDataSelection::OutlinerTableDataSelection(Table* pTable) noexcept
		: TableDataSelection(pTable)
	{
	}

	void OutlinerTableDataSelection::SelectAllExpandedEntityRows() noexcept
	{
		const std::vector<Table::TableDataRow> tableDataRows = m_pTable->FlattenTree();
		for (const auto& row : tableDataRows)
		{
			const OutlinerTableData* pOutlinerTableData = static_cast<const OutlinerTableData*>(row.Entry.get());

			if (pOutlinerTableData->GetType() == ETableDataType::Entity)
			{
				if (!IsSelected(row.Entry))
					Select(row.Entry);
			}
		}
	}

	uint32_t OutlinerTableDataSelection::GetNrOfSelectedEntities() const noexcept
	{
		return m_SelectedEntities.size();
	}

	const std::vector<entity>& OutlinerTableDataSelection::GetSelectedEntities() const noexcept
	{
		return m_SelectedEntities;
	}

	void OutlinerTableDataSelection::DeselectAllEntities() noexcept
	{
		const std::vector<std::shared_ptr<TableData>>& selectedEntries = GetSelected();
		if (selectedEntries.empty())
			return;

		int index = 0;
		for (;;)
		{
			if (selectedEntries.size() <= index)
				break;

			const std::shared_ptr<OutlinerTableData> pOutlinerTableData = static_pointer_cast<OutlinerTableData>(selectedEntries[index]);
			if (pOutlinerTableData->GetType() != ETableDataType::Entity)
			{
				continue;
				index++;
			}
			else
				Deselect(pOutlinerTableData);
		}
	}

	void OutlinerTableDataSelection::OnSelected(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		if (std::shared_ptr<OutlinerEntityTableData> pEntityTableData = dynamic_pointer_cast<OutlinerEntityTableData>(pTableData))
		{
			OutlinerTable* pTable = static_cast<OutlinerTable*>(m_pTable);
			pTable->GetScene()->GetEntityManager().Add<SelectedInEditorComponent>(pEntityTableData->GetEntityID());
			m_SelectedEntities.push_back(pEntityTableData->GetEntityID());
		}
	}

	void OutlinerTableDataSelection::OnDeselected(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		if (std::shared_ptr<OutlinerEntityTableData> pEntityTableData = dynamic_pointer_cast<OutlinerEntityTableData>(pTableData))
		{
			OutlinerTable* pTable = static_cast<OutlinerTable*>(m_pTable);
			pTable->GetScene()->GetEntityManager().Remove<SelectedInEditorComponent>(pEntityTableData->GetEntityID());
			m_SelectedEntities.erase(std::remove(m_SelectedEntities.begin(), m_SelectedEntities.end(), pEntityTableData->GetEntityID()), m_SelectedEntities.end());
		}
	}

	void OutlinerTableDataSelection::OnClicked(const std::shared_ptr<TableData>& pTableData, uint32_t column, bool doubleClicked) noexcept
	{
		OutlinerTableData* pOutlinerTableData = static_cast<OutlinerTableData*>(pTableData.get());
		switch (column)
		{
		case 0:
			pOutlinerTableData->SetAndPropagateVisibleState(!pOutlinerTableData->IsVisible());
			break;
		case 1:
		case 2:
		{
			if (pOutlinerTableData->GetType() == ETableDataType::Folder)
			{
				if (doubleClicked)
					pOutlinerTableData->SetExpanded(!pOutlinerTableData->IsExpanded());
			}
			break;
		}
		}
	}

	bool OutlinerTableDataSelection::IsSelectable(const std::shared_ptr<TableData>& tableData, uint32_t column, SelectionMode selectionMode) noexcept
	{
		switch (column)
		{
		case 0: return false;
		default: return true;
		}
	}
}

