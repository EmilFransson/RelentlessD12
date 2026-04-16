#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	class RLS_API LinearArena
	{
	public:
		LinearArena() = default;

		void Init(void* aMemory, size_t aCapacityBytes) noexcept
		{
			m_pMemory = static_cast<uint8*>(aMemory);
			m_Capacity = aCapacityBytes;
			m_Offset = 0;
		}

		void* Allocate(size_t aBytes, size_t aAlignment = alignof(std::max_align_t)) noexcept
		{
			size_t aligned = (m_Offset + aAlignment - 1) & ~(aAlignment - 1);

			RLS_ASSERT(aligned + aBytes <= m_Capacity, "[LinearArenaAllocator::Allocate]: LinearArena exhausted");
			if (aligned + aBytes > m_Capacity)
				return nullptr;

			void* ptr = m_pMemory + aligned;
			m_Offset = aligned + aBytes;
			return ptr;
		}

		// Deallocation is intentionally a no-op.
		// Memory is reclaimed only by Reset().
		void Deallocate(void*, size_t) noexcept {}

		void Reset() noexcept { m_Offset = 0; }

		NO_DISCARD size_t UsedBytes()      const noexcept { return m_Offset; }
		NO_DISCARD size_t CapacityBytes()  const noexcept { return m_Capacity; }

	private:
		uint8* m_pMemory = nullptr;
		size_t   m_Capacity = 0u;
		size_t   m_Offset = 0u;
	};
}