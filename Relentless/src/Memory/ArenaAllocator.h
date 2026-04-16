#pragma once
#include "LinearArena.h"

namespace Relentless
{
	template<typename T>
	class ArenaAllocator
	{
	public:
		// STL requires this typedef
		using value_type = T;

		// Construct from an arena — the allocator holds a non-owning pointer
		explicit ArenaAllocator(LinearArena& aArena) noexcept
			: m_pArena(&aArena) {}

		// STL requires rebind constructor — allows vector internals to
		// construct ArenaAllocator<U> from ArenaAllocator<T>
		template<typename U>
		ArenaAllocator(const ArenaAllocator<U>& aOther) noexcept
			: m_pArena(aOther.m_pArena) {}

		T* allocate(size_t n) noexcept
		{
			void* ptr = m_pArena->Allocate(sizeof(T) * n, alignof(T));
			return static_cast<T*>(ptr);
		}

		void deallocate(T*, size_t) noexcept
		{
			// No-op — arena reclaims everything at Reset()
		}

		// Equality — two allocators are equal if they share the same arena
		bool operator==(const ArenaAllocator& aOther) const noexcept
		{
			return m_pArena == aOther.m_pArena;
		}

		bool operator!=(const ArenaAllocator& aOther) const noexcept
		{
			return !(*this == aOther);
		}

		// Needs to be accessible for the rebind constructor
		template<typename U> friend class ArenaAllocator;

	private:
		LinearArena* m_pArena = nullptr;
	};
}
