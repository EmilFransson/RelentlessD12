#include "ScratchAllocator.h"
#include "Device.h"
#include "Buffer.h"
#include "CommandContext.h"

namespace Relentless
{
	ScratchAllocationManager::ScratchAllocationManager(GraphicsDevice* pParent, uint64 pageSize) noexcept
		: DeviceObject(pParent), m_PageSize(pageSize)
	{
	}

	Ref<Buffer> ScratchAllocationManager::AllocatePage() noexcept
	{
		std::lock_guard guard(m_AllocationMutex);
		
		Ref<Buffer> pPage = nullptr;
		if (!m_PagePool.empty() && m_PagePool.front().SyncPoint.IsComplete())
		{
			pPage = m_PagePool.front().pPage;
			m_PagePool.pop();
		}
		else
		{
			const std::string name = std::format("Dynamic Allocation Buffer ({0} KB)", Math::BytesToKiloBytes * m_PageSize);
			pPage = GetParent()->CreateBuffer(BufferDesc{ .Size = m_PageSize, .Flags = BufferFlag::Upload }, "Page");
		}

		return pPage;
	}

	void ScratchAllocationManager::FreePages(const SyncPoint& syncPoint, const std::vector<Ref<Buffer>>& pPages) noexcept
	{
		std::lock_guard guard(m_AllocationMutex);
		for (auto pPage : pPages)
			m_PagePool.push(FencedPage(std::move(pPage), syncPoint));
	}

	ScratchAllocator::ScratchAllocator(ScratchAllocationManager* pPageManager) noexcept
		: m_pPageManager(pPageManager)
	{
	}

	ScratchAllocation ScratchAllocator::Allocate(uint64 size, int alignment) noexcept
	{
		const uint64 bufferSize = Math::AlignUp<uint64>(size, alignment);
		ScratchAllocation allocation;
		allocation.Size = bufferSize;

		if (bufferSize > m_pPageManager->GetPageSize())
		{
			Ref<Buffer> pPage = m_pPageManager->GetParent()->CreateBuffer(BufferDesc{ .Size = size, .Flags = BufferFlag::Upload }, "Large Page");
			allocation.Offset = 0;
			allocation.GpuHandle = pPage->GetGpuHandle();
			allocation.pBackingResource = pPage;
			allocation.pMappedMemory = pPage->GetMappedData();
		}
		else
		{
			m_CurrentOffset = Math::AlignUp<uint64>(m_CurrentOffset, alignment);

			if (m_pCurrentPage == nullptr || m_CurrentOffset + bufferSize >= m_pCurrentPage->GetSize())
			{
				m_pCurrentPage = m_pPageManager->AllocatePage();
				m_CurrentOffset = 0;
				m_UsedPages.push_back(m_pCurrentPage);
			}
			allocation.Offset = m_CurrentOffset;
			allocation.GpuHandle = m_pCurrentPage->GetGpuHandle() + m_CurrentOffset;
			allocation.pBackingResource = m_pCurrentPage;
			allocation.pMappedMemory = (char*)m_pCurrentPage->GetMappedData() + m_CurrentOffset;

			m_CurrentOffset += bufferSize;
		}
		return allocation;
	}

	void ScratchAllocator::Free(const SyncPoint& syncPoint) noexcept
	{
		m_pPageManager->FreePages(syncPoint, m_UsedPages);
		m_UsedPages.clear();

		m_pCurrentPage = nullptr;
		m_CurrentOffset = 0;
	}
}