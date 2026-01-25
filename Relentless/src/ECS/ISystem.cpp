#include "ISystem.h"

namespace Relentless
{
	float ISystem::GetTickRate() const noexcept
	{
		return m_TickRate;
	}

	void ISystem::SetTickRate(const float aTickRate) noexcept
	{
		m_TickRate = aTickRate;
	}

}