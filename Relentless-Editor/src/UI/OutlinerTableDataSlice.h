#pragma once
#include <Relentless.h>

namespace Relentless
{
	class OutlinerTableDataSlice : public TableDataSlice
	{
	public:
		OutlinerTableDataSlice(Table* table, TableData* pOwner) noexcept;
		virtual ~OutlinerTableDataSlice() noexcept override = default;

	};
}