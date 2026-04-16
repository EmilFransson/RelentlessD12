#pragma once
#include "Core/Application.h"

#include "ArenaAllocator.h"

namespace Relentless
{
	template<typename T>
	class ArenaVector : public std::vector<T, ArenaAllocator<T>>
	{
	public:
		explicit ArenaVector(size_t aReserveCount = 0) noexcept
			: std::vector<T, ArenaAllocator<T>>(ArenaAllocator<T>(g_FrameScratchArena))
		{
			if (aReserveCount > 0)
				this->reserve(aReserveCount);
		}
	};
}