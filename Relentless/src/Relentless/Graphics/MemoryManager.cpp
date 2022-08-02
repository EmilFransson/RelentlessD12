#include "MemoryManager.h"
#include "D3D12Core.h"
#include "../Window.h"
namespace Relentless
{
	MemoryManager MemoryManager::s_Instance;

	MemoryManager& MemoryManager::Get() noexcept
	{
		return s_Instance;
	}

	void MemoryManager::Initialize() noexcept
	{
		m_pRTVDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100'000, false));
		m_pShaderBindablesDescriptorHeapNV = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, false));
		m_pDeferredFreeLists = std::move(std::unique_ptr<std::vector<DescriptorHandle>[]>(new std::vector<DescriptorHandle>[D3D12Core::GetNrOfBufferedFrames()]));
	}

	const DescriptorHandle MemoryManager::CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept
	{
		switch (descriptorHandleType)
		{
		case DescriptorHandleType::RTV:
		{
			DescriptorHandle handle = m_pRTVDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::SRV_NV:
		case DescriptorHandleType::CBV_NV:
		case DescriptorHandleType::UAV_NV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		default:
			RLS_ASSERT(false, "Unknown dscriptor handle type.");
			break;
		}
		return DescriptorHandle();
	}

	void MemoryManager::DestroyDescriptorHandle(const DescriptorHandle& descriptorHandle) noexcept
	{
		const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		m_pDeferredFreeLists[index].emplace_back(descriptorHandle);
	}

	void MemoryManager::PerformDeferredDeletion() noexcept
	{
		const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		if (m_pDeferredFreeLists[index].empty())
			return;

		for (uint32_t i{ 0u }; i < m_pDeferredFreeLists[index].size(); ++i)
		{
			switch (m_pDeferredFreeLists[index][i].Type)
			{
			case DescriptorHandleType::RTV:
				m_pRTVDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[index][i]);
				break;
			case DescriptorHandleType::CBV_NV:
			case DescriptorHandleType::SRV_NV:
			case DescriptorHandleType::UAV_NV:
				m_pShaderBindablesDescriptorHeapNV->FreeDescriptor(m_pDeferredFreeLists[index][i]);
				break;
			}
		}
		m_pDeferredFreeLists[index].clear();
	}
}