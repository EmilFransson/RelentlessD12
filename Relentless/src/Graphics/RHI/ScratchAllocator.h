#pragma once
#include "Buffer.h"
#include "Fence.h"

namespace Relentless
{
	struct ScratchAllocation
	{
		Ref<Buffer> pBackingResource = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS GpuHandle{ 0 };
		uint64 Offset = 0;
		uint64 Size = 0;
		void* pMappedMemory = nullptr;
		void Clear(uint32 value = 0)
		{
			memset(pMappedMemory, value, Size);
		}

		template<typename T>
		T& As()
		{
			RLS_ASSERT(sizeof(T) <= Size, "Cast Type Size Exceeds Allocation Size.");
			return *static_cast<T*>(pMappedMemory);
		}
	};

	class ScratchAllocationManager : public DeviceObject
	{
	public:
		ScratchAllocationManager(GraphicsDevice* pParent, uint64 pageSize) noexcept;
		virtual ~ScratchAllocationManager() noexcept override = default;

		[[nodiscard]] Ref<Buffer> AllocatePage() noexcept;
		void FreePages(const SyncPoint& syncPoint, const std::vector<Ref<Buffer>>& pPages) noexcept;
		[[nodiscard]] uint64 GetPageSize() const noexcept { return m_PageSize; }

	private:
		struct FencedPage
		{
			FencedPage(Ref<Buffer> pBuffer, const SyncPoint& syncPoint) noexcept
				: pPage{ pBuffer }, SyncPoint{ syncPoint } {}

			Ref<Buffer> pPage = nullptr;
			SyncPoint SyncPoint;
		};

		uint64 m_PageSize = 0u;
		std::mutex m_AllocationMutex;
		std::queue<FencedPage> m_PagePool;
	};

	class ScratchAllocator
	{
	public:
		ScratchAllocator(ScratchAllocationManager* pPageManager) noexcept;
		ScratchAllocation Allocate(uint64 size, int alignment) noexcept;
		void Free(const SyncPoint& syncPoint) noexcept;

	private:
		ScratchAllocationManager* m_pPageManager = nullptr;

		Ref<Buffer> m_pCurrentPage = nullptr;
		uint64 m_CurrentOffset = 0;
		std::vector<Ref<Buffer>> m_UsedPages;
	};
}