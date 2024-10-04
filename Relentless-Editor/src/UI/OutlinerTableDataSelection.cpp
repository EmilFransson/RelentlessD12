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
		for (auto& row : tableDataRows)
		{
			if (!dynamic_pointer_cast<OutlinerEntityTableData>(row.Entry))
				continue;

			if (!IsSelected(row.Entry) && IsSelectable(row.Entry, TableDataSelection::SelectionMode::Range))
				Select(row.Entry);
		}
	}

	uint32_t OutlinerTableDataSelection::GetNrOfSelectedEntities() const noexcept
	{
		return m_NrOfSelectedEntities;
	}

	void OutlinerTableDataSelection::OnSelected(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		if (std::shared_ptr<OutlinerEntityTableData> pEntityTableData = dynamic_pointer_cast<OutlinerEntityTableData>(pTableData))
			m_NrOfSelectedEntities++;
	}

	void OutlinerTableDataSelection::OnDeselected(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		if (std::shared_ptr<OutlinerEntityTableData> pEntityTableData = dynamic_pointer_cast<OutlinerEntityTableData>(pTableData))
			m_NrOfSelectedEntities--;
	}

	void OutlinerTableDataSelection::OnClicked(const std::shared_ptr<TableData>& pTableData, uint32_t column, bool doubleClicked) noexcept
	{
		if (std::shared_ptr<OutlinerSceneTableData> pSceneTableData = dynamic_pointer_cast<OutlinerSceneTableData>(pTableData))
			OnSceneRowClicked(pSceneTableData, column);
		else if (std::shared_ptr<OutlinerEntityTableData> pEntityTableData = dynamic_pointer_cast<OutlinerEntityTableData>(pTableData))
			OnEntityRowClicked(pEntityTableData, column, doubleClicked);
	}

	void OutlinerTableDataSelection::OnSceneRowClicked(const std::shared_ptr<OutlinerSceneTableData>& pSceneTableData, uint32_t column) noexcept
	{
		if (column == 0)
		{
			Scene* pScene = pSceneTableData->GetScene();
			EntityManager& entityManager = pScene->GetEntityManager();

			const bool sceneIsHidden = entityManager.GetEntityCountForPool<HiddenInGameComponent>() == entityManager.GetEntityAliveCount();

			if (sceneIsHidden)
			{
				entityManager.Collect<IDComponent>().Do([&](entity e)
					{
						if (entityManager.Has<HiddenInGameComponent>(e))
							entityManager.Remove<HiddenInGameComponent>(e);
					});
			}
			else
			{
				entityManager.Collect<IDComponent>().Do([&](entity e)
					{
						entityManager.AddOrReplace<HiddenInGameComponent>(e);
					});
			}
		}
	}

	void OutlinerTableDataSelection::OnEntityRowClicked(const std::shared_ptr<OutlinerEntityTableData>& pEntityTableData, uint32_t column, bool doubleClicked) noexcept
	{
		if (column == 0)
		{
			OutlinerTable* pTable = static_cast<OutlinerTable*>(m_pTable);
			Scene* pScene = pTable->GetScene();
			EntityManager& entityManager = pScene->GetEntityManager();
			const entity e = pEntityTableData->GetEntityID();

			if (entityManager.Has<HiddenInGameComponent>(e))
				entityManager.Remove<HiddenInGameComponent>(e);
			else
				entityManager.Add<HiddenInGameComponent>(e);
		}
	}

}

