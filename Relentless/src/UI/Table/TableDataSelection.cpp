#include "TableDataSelection.h"

#include "Table.h"
#include "TableData.h"
#include "TableDataSlice.h"

namespace Relentless
{
	TableDataSelection::TableDataSelection(Table* pTable) noexcept
		: m_pTable{ pTable }
	{

	}

	bool TableDataSelection::IsReferenceSelection(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		return m_pReference == pTableData;
	}

	void TableDataSelection::OnDrawBegin() noexcept
	{
		m_HoveredStateSetThisFrame = false;
	}

	void TableDataSelection::OnDrawEnd() noexcept
	{
		if (!m_HoveredStateSetThisFrame)
			m_pHovered = nullptr;
	}

	void TableDataSelection::OnClickedOnRow(const std::shared_ptr<TableData>& pTableData, SelectionMode selectionMode, uint32_t column, bool doubleClicked) noexcept
	{
		switch (selectionMode)
		{
		case SelectionMode::Single:
		{
			if (GetSelectedCount() > 1 && IsSelected(pTableData))
				return;

			if (IsSelectable(pTableData, SelectionMode::Single))
			{
				DeselectAll();
				Select(pTableData);
				SetReferenceSelection(pTableData);
			}
			OnClicked(pTableData, column, doubleClicked);

			break;
		}
		case SelectionMode::Toggle:
		{
			if (IsSelected(pTableData))
			{
				Deselect(pTableData);
				if (GetSelectedCount() > 0)
					SetReferenceSelection(m_Selected.front());
			}
			else
			{
				if (IsSelectable(pTableData, SelectionMode::Toggle))
				{
					Select(pTableData);
					SetReferenceSelection(pTableData);
				}
			}

			break;
		}
		case SelectionMode::Range:
		{
			const std::vector<Table::TableDataRow> tableDatas = m_pTable->FlattenTree();
			if (tableDatas.empty())
				break;

			size_t startIndex = 0u;
			if (m_pReference)
			{
				auto it = std::find_if(tableDatas.begin(), tableDatas.end(), [&](const Table::TableDataRow& row)
					{
						return row.Entry == m_pReference;
					});
				startIndex = std::distance(tableDatas.begin(), it);
			}
			else
				m_pReference = tableDatas[0].Entry;

			auto it = std::find_if(tableDatas.begin(), tableDatas.end(), [&](const Table::TableDataRow& row)
				{
					return row.Entry == pTableData;
				});

			size_t endIndex = std::distance(tableDatas.begin(), it);

			if (startIndex > endIndex)
				std::swap(startIndex, endIndex);
			
			for (size_t i = startIndex; i <= endIndex; ++i)
			{
				if (IsSelectable(tableDatas[i].Entry, SelectionMode::Range) && !IsSelected(tableDatas[i].Entry))
					Select(tableDatas[i].Entry);
			}

			break;
		}
		}
	}

	void TableDataSelection::OnReleasedOnRow(const std::shared_ptr<TableData>& tableData, SelectionMode selectionMode) noexcept
	{
		if (!IsSelected(tableData))
			return;

		if (GetSelectedCount() <= 1)
			return;

		switch (selectionMode)
		{
		case SelectionMode::Single:
		{
			DeselectAll();
			Select(tableData);
			SetReferenceSelection(tableData);
			break;
		}
		}
	}

	void TableDataSelection::Select(const std::shared_ptr<TableData>& tableData)
	{
		m_Selected.push_back(tableData);
		OnSelected(tableData);
	}

	void TableDataSelection::Deselect(const std::shared_ptr<TableData>& tableData) noexcept
	{
		auto it = std::find(m_Selected.begin(), m_Selected.end(), tableData);
		OnDeselected(*it);
	
		m_Selected.erase(it);
	}

	void TableDataSelection::DeselectAll() noexcept
	{
		for (auto& selected : m_Selected)
			OnDeselected(selected);

		m_Selected.clear();
	}

	bool TableDataSelection::IsSelected(const std::shared_ptr<TableData>& tableData) const noexcept
	{
		return std::any_of(m_Selected.begin(), m_Selected.end(), [&](const std::shared_ptr<TableData>& pCurrentTableData) -> bool
			{;
				return pCurrentTableData == tableData;
			});
	}

	uint32_t TableDataSelection::GetSelectedCount() const noexcept
	{
		return m_Selected.size();
	}

	void TableDataSelection::SetHovered(const std::shared_ptr<TableData>& tableData) noexcept
	{
		m_pHovered = tableData;
		m_HoveredStateSetThisFrame = true;

		OnHovered(m_pHovered);
	}

	bool TableDataSelection::IsHovered(const std::shared_ptr<TableData>& tableData) const noexcept
	{
		return m_pHovered && m_pHovered == tableData;
	}

	bool TableDataSelection::IsAncestorToAnySelected(const std::shared_ptr<TableData>& tableData) const noexcept
	{
		std::vector<std::shared_ptr<TableData>> decendants;

		std::function<void(const std::shared_ptr<TableData>&)> GetChildren;

		GetChildren = [&](const std::shared_ptr<TableData>& currentTableData)
			{
				if (!currentTableData->HasChildren())
					return;

				const std::unique_ptr<TableDataSlice>& pSlice = currentTableData->GetSlice();
				const std::vector<std::shared_ptr<TableData>> children = pSlice->GetData();
				for (auto& child : children)
				{
					decendants.push_back(child);
					GetChildren(child);
				}
			};

		GetChildren(tableData);

		return std::any_of(m_Selected.begin(), m_Selected.end(), [&](const std::shared_ptr<TableData>& pCurrentTableData) -> bool
			{
				return std::any_of(decendants.begin(), decendants.end(), [&pCurrentTableData](const std::shared_ptr<TableData>& pCurrentData)
					{
						return pCurrentTableData == pCurrentData;
					});
			});
	}

	void TableDataSelection::SetReferenceSelection(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		m_pReference = pTableData;
	}

}


