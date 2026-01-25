#pragma once

namespace Relentless
{
	namespace Process
	{
		NO_DISCARD __forceinline unsigned long GetCurrentID() noexcept
		{
			return ::GetCurrentProcessId();
		}
	}
}