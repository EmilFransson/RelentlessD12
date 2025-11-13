#include "IDetailsView.h"

namespace Relentless
{
	bool IDetailsView::IsLocked() const noexcept
	{
		return m_IsLocked;
	}
}