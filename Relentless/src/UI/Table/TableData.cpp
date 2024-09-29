#include "TableData.h"
#include "TableDataSlice.h"

namespace Relentless
{
	TableData::TableData() noexcept
	{
	}

	bool TableData::HasChildren() const noexcept
	{
		return m_pSlice && m_pSlice->GetData().size() > 0;
	}

}