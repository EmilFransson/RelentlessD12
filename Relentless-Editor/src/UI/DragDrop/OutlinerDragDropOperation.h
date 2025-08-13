#pragma once

#include <Relentless.h>

namespace Relentless
{
	class OutlinerDragDropOperation : public DragDropOperation
	{
	public:
		static constexpr const char* kTypeName	= "ENTITY_DRAG_DROP_OP";
		static constexpr uint64      kType		= "ENTITY_DRAG_DROP_OP"_h;

		NO_DISCARD uint64 GetType() const noexcept override { return kType; }
		NO_DISCARD const char* GetTypeName() const noexcept override { return kTypeName; }
	};
}