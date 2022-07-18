#include "DescriptorHeap.h"
#include "D3D12Core.h"
namespace Relentless
{
	DescriptorHeap::DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, const uint32_t capacity, const bool isShaderVisible) noexcept
		: m_Type{descriptorHeapType},
		  m_Capacity{capacity},
		  m_CurrentNrOfDescriptors{0u}
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		RLS_ASSERT(!(descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV && isShaderVisible == true), "RTV-descriptor heap type does not support shader visibility.");
		RLS_ASSERT(!(descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV && isShaderVisible == true), "RTV-descriptor heap type does not support shader visibility.");
		RLS_ASSERT(!(capacity > D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1), "Capacity exceeds maximum shader tier (1) supported capacity.");
		RLS_ASSERT(!(descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE), "Capacity exceeds maximum supported shader sampler heap size (2048).");
		
		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescriptor{};
		descriptorHeapDescriptor.Type = descriptorHeapType;
		descriptorHeapDescriptor.NumDescriptors = capacity;
		descriptorHeapDescriptor.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescriptor.NodeMask = 0u;
		DXCall(D3D12Core::GetDevice()->CreateDescriptorHeap(&descriptorHeapDescriptor, IID_PPV_ARGS(&m_pDescriptorHeap)));

		m_FreeHandles = std::move(std::make_unique<uint32_t[]>(capacity));
		for (uint32_t i{ 0u }; i < m_Capacity; ++i)
		{
			m_FreeHandles[i] = i;
		}

		DXCall_STD(m_CpuHandleStart = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		if (isShaderVisible)
		{
			DXCall_STD(m_GpuHandleStart = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		}
		else
		{
			m_GpuHandleStart.ptr = 0u;
		}

		DXCall_STD(m_DescriptorSize = D3D12Core::GetDevice()->GetDescriptorHandleIncrementSize(m_Type));
	}

	DescriptorHandle DescriptorHeap::AllocateDescriptor() noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		RLS_ASSERT(m_pDescriptorHeap, "D3D12 Descriptor heap interface is not initialized.");
		RLS_ASSERT(m_CurrentNrOfDescriptors != m_Capacity, "Descriptor heap capacity reached.");

		//Perhaps check a free-list for earlier deleted (and now free!) handles?

		const uint32_t index = m_FreeHandles[m_CurrentNrOfDescriptors];
		const uint32_t offset = index * m_DescriptorSize;
		m_CurrentNrOfDescriptors++;

		DescriptorHandle descriptorHandleToReturn{};
		descriptorHandleToReturn.CPUHandle.ptr = m_CpuHandleStart.ptr + offset;
		if (IsShaderVisible())
		{
			descriptorHandleToReturn.GPUHandle.ptr = m_GpuHandleStart.ptr + offset;
		}

#if defined(RLS_DEBUG)
		descriptorHandleToReturn.pDebugInterface = this;
		descriptorHandleToReturn.Index = index;
#endif
		return descriptorHandleToReturn; 
	}

	void DescriptorHeap::FreeDescriptor(const DescriptorHandle& descriptorHandle) noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		RLS_ASSERT(m_pDescriptorHeap, "D3D12 Descriptor heap interface is not initialized.");
		RLS_ASSERT(descriptorHandle.pDebugInterface == this, "Descriptor heap object pointer mismatch.");
		RLS_ASSERT(descriptorHandle.Index <= m_Capacity, "Descriptor handle index out of bounds.");
		RLS_ASSERT(descriptorHandle.CPUHandle.ptr > 0 && (descriptorHandle.CPUHandle.ptr <= m_CpuHandleStart.ptr + (m_CurrentNrOfDescriptors * m_DescriptorSize)), "CPU descriptor handle out of bounds.");
		RLS_ASSERT(!(descriptorHandle.GPUHandle.ptr > 0 && descriptorHandle.GPUHandle.ptr <= m_GpuHandleStart.ptr + (m_CurrentNrOfDescriptors * m_DescriptorSize)), "GPU descriptor handle out of bounds.");
		RLS_ASSERT((descriptorHandle.CPUHandle.ptr - m_CpuHandleStart.ptr) % m_DescriptorSize == 0u, "CPU handle pointer is not valid for this descriptor heap.");

		//TODO: Handle deferred freeing of descriptors.
		m_DeferredFreeList.emplace_back(descriptorHandle);
	}
}