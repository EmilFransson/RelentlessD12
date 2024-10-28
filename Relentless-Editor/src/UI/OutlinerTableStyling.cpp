#include "OutlinerTableStyling.h"
#include "OutlinerTableData.h"

namespace Relentless
{
	OutlinerTableStyling::OutlinerTableStyling(TableDataSelection* pTableDataSelection, Scene* pScene) noexcept
		:
		TableStyling(pTableDataSelection),
		m_pScene{pScene}
	{
	}

	const TableRowStyle OutlinerTableStyling::GetRowStyle(const std::shared_ptr<TableData>& pTableData, uint32_t column) const noexcept
	{
		TableRowStyle style = m_DefaultRowStyle;

		if (column == 0u)
		{
			style.Alignment = UI::Alignment::Center;
			const bool selected = m_pTableDataSelection->IsSelected(pTableData);
			const bool anyColumnHovered = m_pTableDataSelection->IsHovered(pTableData);
			const bool hoversVisibilityColumn = m_pTableDataSelection->IsHovered(pTableData, 0);
			
			bool visible = true;
			if (std::shared_ptr<OutlinerEntityTableData> pEntityTableData = dynamic_pointer_cast<OutlinerEntityTableData>(pTableData))
			{
				const entity entityID = pEntityTableData->GetEntityID();
				visible = !m_pScene->GetEntityManager().Has<HiddenInGameComponent>(entityID);
			}
			else if (dynamic_pointer_cast<OutlinerSceneTableData>(pTableData))
				visible = !(m_pScene->GetEntityManager().GetEntityCountForPool<HiddenInGameComponent>() == m_pScene->GetEntityManager().GetEntityAliveCount());
			else //Folder
				visible = static_pointer_cast<OutlinerFolderTableData>(pTableData)->IsVisible();

			style.IconTint = (selected || hoversVisibilityColumn) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : (anyColumnHovered || !visible) ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f);
		}

		return style;
	}
}
