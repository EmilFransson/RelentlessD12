#include "OutlinerDragDropOperation.h"

#include "../OutlinerTableRow.h"
#include "../../Core/EntityFolders.h"

namespace Relentless
{
	OutlinerDragDropOperation::OutlinerDragDropOperation(OutlinerTableRow* pDragStartRow) noexcept
		: m_pDragStartRow{pDragStartRow}
	{
	}

	const std::vector<entity>& OutlinerDragDropOperation::GetDraggedEntities() const noexcept
	{
		return m_DraggedEntities;
	}

	const std::vector<EntityFolder*>& OutlinerDragDropOperation::GetDraggedFolders() const noexcept
	{
		return m_DraggedFolders;
	}

	OutlinerTableRow* OutlinerDragDropOperation::GetDragInitiatorRow() const noexcept
	{
		return m_pDragStartRow;
	}

	void OutlinerDragDropOperation::SetDraggedEntities(const std::vector<entity>& someEntities) noexcept
	{
		m_DraggedEntities = someEntities;
	}

	void OutlinerDragDropOperation::SetDraggedFolders(const std::vector<EntityFolder*>& someFolders) noexcept
	{
		m_DraggedFolders = someFolders;
	}
}
