#pragma once

#include <Relentless.h>

namespace Relentless
{
	class EntityFilter;
	class OutlinerTableRow;

	class OutlinerDragDropOperation : public DragDropOperation
	{
	public:
		OutlinerDragDropOperation(OutlinerTableRow* pDragStartRow) noexcept;

		static constexpr const char* kTypeName	= "ENTITY_DRAG_DROP_OP";
		static constexpr uint64      kType		= "ENTITY_DRAG_DROP_OP"_h;

		NO_DISCARD const std::vector<entity>& GetDraggedEntities() const noexcept;
		NO_DISCARD const std::vector<EntityFilter*>& GetDraggedFilters() const noexcept;
		NO_DISCARD OutlinerTableRow* GetDragInitiatorRow() const noexcept;

		NO_DISCARD uint64 GetType() const noexcept override { return kType; }
		NO_DISCARD const char* GetTypeName() const noexcept override { return kTypeName; }

		void SetDraggedEntities(const std::vector<entity>& entities) noexcept;
		void SetDraggedFilters(const std::vector<EntityFilter*>& filters) noexcept;
	private:
		std::vector<entity> m_DraggedEntities;
		std::vector<EntityFilter*> m_DraggedFilters;

		OutlinerTableRow* m_pDragStartRow = nullptr;
	};
}