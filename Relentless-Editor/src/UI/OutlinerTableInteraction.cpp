#include "OutlinerTableInteraction.h"
#include "OutlinerTableData.h"

namespace Relentless
{

	OutlinerTableInteraction::OutlinerTableInteraction(Table* pTable) noexcept
		: TableInteraction{ pTable }
	{
	}

	bool OutlinerTableInteraction::IsDraggable(const std::shared_ptr<TableData>& pTableData, uint32_t column) const noexcept
	{
		if (column == 0u || dynamic_pointer_cast<OutlinerSceneTableData>(pTableData))
			return false;
		else
			return true;
	}

	TableInteraction::PayloadInfo OutlinerTableInteraction::GetPayloadInfo(const std::vector<std::shared_ptr<TableData>>& selectedRows) const noexcept
	{
		return PayloadInfo
		{
			.ID = "OUTLINER_TABLE_DRAGDROP",
			.Data = static_cast<const void*>(&m_Payload),
			.Size = sizeof(int),
			.TooltipLabel = "Dragging int"
		};
	}
}
