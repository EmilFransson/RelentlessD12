#include "OutlinerDragDropOperation.h"

#include "../OutlinerTableRow.h"
#include "../../Core/EntityFilters.h"

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

	const std::vector<EntityFilter*>& OutlinerDragDropOperation::GetDraggedFilters() const noexcept
	{
		return m_DraggedFilters;
	}

	OutlinerTableRow* OutlinerDragDropOperation::GetDragInitiatorRow() const noexcept
	{
		return m_pDragStartRow;
	}

	void OutlinerDragDropOperation::SetDraggedEntities(const std::vector<entity>& entities) noexcept
	{
		m_DraggedEntities = entities;
	}

	void OutlinerDragDropOperation::SetDraggedFilters(const std::vector<EntityFilter*>& filters) noexcept
	{
		m_DraggedFilters = filters;
	}

}
