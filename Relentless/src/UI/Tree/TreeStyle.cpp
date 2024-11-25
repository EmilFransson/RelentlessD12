#include "TreeStyle.h"

namespace Relentless
{
	void TreeStyle::SetUseAlternatingRowColors(bool useState) noexcept
	{
		m_UseAlternatingColors = useState;
	}

	bool TreeStyle::IsUsingAlternateRowColors() const noexcept
	{
		return m_UseAlternatingColors;
	}

}