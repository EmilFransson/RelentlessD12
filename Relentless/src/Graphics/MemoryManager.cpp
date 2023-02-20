#include "MemoryManager.h"
#include "D3D12Core.h"
#include "../Core/Window.h"
#include "Resources/ConstantBuffer.h"
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
		m_pDSVDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 10, false));
		m_pShaderBindablesDescriptorHeapNV = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, false));
		m_pShaderBindablesDescriptorHeap = std::move(std::make_unique<DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, true));
		m_pDeferredFreeLists = std::move(std::unique_ptr<std::vector<DescriptorHandle>[]>(RLS_NEW std::vector<DescriptorHandle>[D3D12Core::GetNrOfBufferedFrames()]));
		m_pDeferredFreeListsResources = std::move(std::unique_ptr<std::vector<std::shared_ptr<IResource>>[]>(RLS_NEW std::vector<std::shared_ptr<IResource>>[D3D12Core::GetNrOfBufferedFrames()]));
		m_pUploadBuffer = std::move(std::make_unique<UploadBuffer>(300'000, "Main Upload Buffer"));
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
		case DescriptorHandleType::DSV:
		{
			DescriptorHandle handle = m_pDSVDescriptorHeap->AllocateDescriptor();
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
		case DescriptorHandleType::SRV:
		case DescriptorHandleType::CBV:
		case DescriptorHandleType::UAV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		default:
			RLS_ASSERT(false, "Unknown descriptor handle type.");
			break;
		}
		return DescriptorHandle();
	}

	void MemoryManager::DestroyDescriptorHandle(const DescriptorHandle& descriptorHandle) noexcept
	{
		//const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		const uint32_t frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();

		m_pDeferredFreeLists[frameIndex].emplace_back(descriptorHandle);
	}

	void MemoryManager::DestroyResource(std::shared_ptr<IResource> pResource) noexcept
	{
		//const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		const uint32_t frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();

		
		m_pDeferredFreeListsResources[frameIndex].emplace_back(std::move(pResource));
	}

	void MemoryManager::PerformDeferredDeletion() noexcept
	{
		//const uint32_t index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		const uint32_t frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();
		
		if (!m_pDeferredFreeLists[frameIndex].empty())
		{
			for (uint32_t i{ 0u }; i < m_pDeferredFreeLists[frameIndex].size(); ++i)
			{
				switch (m_pDeferredFreeLists[frameIndex][i].Type)
				{
				case DescriptorHandleType::RTV:
					m_pRTVDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
					break;
				case DescriptorHandleType::DSV:
					m_pDSVDescriptorHeap->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
					break;
				case DescriptorHandleType::CBV_NV:
				case DescriptorHandleType::SRV_NV:
				case DescriptorHandleType::UAV_NV:
					m_pShaderBindablesDescriptorHeapNV->FreeDescriptor(m_pDeferredFreeLists[frameIndex][i]);
					break;
				}
			}
			m_pDeferredFreeLists[frameIndex].clear();
		}
#if defined (RLS_DEBUG)
		for (uint32_t i{ 0u }; i < m_pDeferredFreeListsResources[frameIndex].size(); i++)
			RLS_CORE_WARN("Destroyed resource '{0}'", m_pDeferredFreeListsResources[frameIndex][i]->GetName());
#endif
		if (!m_pDeferredFreeListsResources[frameIndex].empty())
			m_pDeferredFreeListsResources[frameIndex].clear();
	}

	void MemoryManager::UpdateConstantBuffer(const ConstantBuffer& constantBuffer, void* pData) noexcept
	{
		RLS_ASSERT(pData, "Memory address to copy from is nullptr.");
		auto address = constantBuffer.GetInterface()->GetGPUVirtualAddress();

		constexpr const D3D12_RANGE range = { 0,0 };
		DXCall(constantBuffer.GetInterface()->Map(0u, &range, reinterpret_cast<void**>(&address)));
		std::memcpy(reinterpret_cast<void*>(address), reinterpret_cast<unsigned char*>(pData), constantBuffer.m_SizeInBytes);
		DXCall_STD(constantBuffer.GetInterface()->Unmap(0u, nullptr));

		//auto index = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		auto frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();
		
		auto dstHandle = constantBuffer.m_VisibleHandles[frameIndex].CPUHandle;
		auto srcHandle = constantBuffer.m_NonVisibleHandle.CPUHandle;

		DXCall_STD(D3D12Core::GetDevice()->CopyDescriptorsSimple(1u, dstHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
	}

	void MemoryManager::UpdateStructuredBuffer(const StructuredBuffer& structuredBuffer, void* pData, uint32_t index) noexcept
	{
		RLS_ASSERT(pData, "Memory address to copy from is nullptr.");
		RLS_ASSERT(index < structuredBuffer.m_Capacity, "Capacity reached.");

		auto address = structuredBuffer.GetInterface()->GetGPUVirtualAddress();

		constexpr const D3D12_RANGE range = { 0,0 };
		DXCall(structuredBuffer.GetInterface()->Map(0u, &range, reinterpret_cast<void**>(&address)));
		address += (index * structuredBuffer.m_ByteStride);
		std::memcpy(reinterpret_cast<void*>(address), reinterpret_cast<unsigned char*>(pData), structuredBuffer.m_ByteStride);
		DXCall_STD(structuredBuffer.GetInterface()->Unmap(0u, nullptr));

		//auto frameIndex = Window::GetCurrentBackbufferIndex() % D3D12Core::GetNrOfBufferedFrames();
		auto frameIndex = D3D12Core::GetCurrentFrame() % D3D12Core::GetNrOfBufferedFrames();
		
		auto dstHandle = structuredBuffer.m_VisibleHandles[frameIndex].CPUHandle;
		auto srcHandle = structuredBuffer.m_NonVisibleHandle.CPUHandle;

		DXCall_STD(D3D12Core::GetDevice()->CopyDescriptorsSimple(1, dstHandle, srcHandle, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV));
		
	}
}