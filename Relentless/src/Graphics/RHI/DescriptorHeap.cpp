#include "DescriptorHeap.h"
#include "Device.h"
#include "D3D.h"

namespace Relentless
{
	DescriptorHeap::DescriptorHeap(GraphicsDevice* pParent, const D3D12_DESCRIPTOR_HEAP_TYPE descriptorHeapType, const uint32_t capacity, const bool isShaderVisible) noexcept
		:
		DeviceObject(pParent),
		m_Type{ descriptorHeapType },
		m_Capacity{ capacity }
	{
		RLS_ASSERT(!(descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV && isShaderVisible == true), "RTV-descriptor heap type does not support shader visibility.");
		RLS_ASSERT(!(descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV && isShaderVisible == true), "RTV-descriptor heap type does not support shader visibility.");
		RLS_ASSERT(!(capacity > D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1), "Capacity exceeds maximum shader tier (1) supported capacity.");
		RLS_ASSERT(!(descriptorHeapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER && capacity > D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE), "Capacity exceeds maximum supported shader sampler heap size (2048).");

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDescriptor{};
		descriptorHeapDescriptor.Type = descriptorHeapType;
		descriptorHeapDescriptor.NumDescriptors = capacity;
		descriptorHeapDescriptor.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		descriptorHeapDescriptor.NodeMask = 1u;
		VERIFY_HR_EX(GetParent()->GetDevice()->CreateDescriptorHeap(&descriptorHeapDescriptor, IID_PPV_ARGS(m_pDescriptorHeap.ReleaseAndGetAddressOf())), GetParent()->GetDevice());

		m_FreeHandles = std::move(std::make_unique<uint32_t[]>(capacity));
		for (uint32_t i{ 0u }; i < m_Capacity; ++i)
			m_FreeHandles[i] = i;

		m_CpuHandleStart = m_pDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		
		if (isShaderVisible)
			m_GpuHandleStart = m_pDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
		else
			m_GpuHandleStart.ptr = 0u;

		m_DescriptorSize = GetParent()->GetDevice()->GetDescriptorHandleIncrementSize(m_Type);
		
		SetDebugName();
	}

	DescriptorHandle DescriptorHeap::AllocateDescriptor() noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		RLS_ASSERT(m_pDescriptorHeap, "D3D12 Descriptor heap interface is not initialized.");
		RLS_ASSERT(m_CurrentNrOfDescriptors != m_Capacity, "Descriptor heap capacity reached.");

		if (!m_FreeList.empty() && m_FreeList.front().SyncPoint.IsComplete())
		{
			const DescriptorHandle descriptorHandle = m_FreeList.front().DescriptorHandle;
			m_FreeList.pop();
			return descriptorHandle;
		}
		else
		{
			m_CurrentNrOfDescriptors++;
			const uint32_t index = m_FreeHandles[m_CurrentNrOfDescriptors];
			const uint32_t offset = index * m_DescriptorSize;

			DescriptorHandle descriptorHandleToReturn{};
			descriptorHandleToReturn.CPUHandle.ptr = m_CpuHandleStart.ptr + offset;
			if (IsShaderVisible())
			{
				descriptorHandleToReturn.GPUHandle.ptr = m_GpuHandleStart.ptr + offset;
			}

			descriptorHandleToReturn.Index = index;
#if defined(RLS_DEBUG)
			descriptorHandleToReturn.pDebugInterface = this;
#endif
			return descriptorHandleToReturn;
		}
	}

	std::vector<DescriptorHandle> DescriptorHeap::AllocateDescriptorBlock(uint32 blockSize) noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);

		RLS_ASSERT(m_pDescriptorHeap, "D3D12 Descriptor heap interface is not initialized.");
		RLS_ASSERT(m_CurrentNrOfDescriptors != m_Capacity, "Descriptor heap capacity reached.");

		std::vector<DescriptorHandle> descriptorBlock;
		descriptorBlock.reserve(blockSize);
			
		for (uint32 i = 0u; i < blockSize; ++i)
		{
			if (!m_FreeList.empty() && m_FreeList.front().SyncPoint.IsComplete())
			{
				const DescriptorHandle descriptorHandle = m_FreeList.front().DescriptorHandle;
				m_FreeList.pop();
				descriptorBlock.push_back(descriptorHandle);
			}
			else
			{
				m_CurrentNrOfDescriptors++;
				const uint32_t index = m_FreeHandles[m_CurrentNrOfDescriptors];
				const uint32_t offset = index * m_DescriptorSize;

				DescriptorHandle descriptorHandle{};
				descriptorHandle.CPUHandle.ptr = m_CpuHandleStart.ptr + offset;
				if (IsShaderVisible())
				{
					descriptorHandle.GPUHandle.ptr = m_GpuHandleStart.ptr + offset;
				}

				descriptorHandle.Index = index;
#if defined(RLS_DEBUG)
				descriptorHandle.pDebugInterface = this;
#endif
				descriptorBlock.push_back(descriptorHandle);
			}
		}

		return descriptorBlock;
	}

	void DescriptorHeap::FreeDescriptor(const DescriptorHandle& descriptorHandle, const SyncPoint& syncPoint) noexcept
	{
		std::lock_guard<std::mutex> lock(m_Mutex);
		RLS_ASSERT(m_pDescriptorHeap, "D3D12 Descriptor heap interface is not initialized.");
		RLS_ASSERT(descriptorHandle.pDebugInterface == this, "Descriptor heap object pointer mismatch.");
		RLS_ASSERT(descriptorHandle.Index <= m_Capacity, "Descriptor handle index out of bounds.");
		RLS_ASSERT((descriptorHandle.CPUHandle.ptr - m_CpuHandleStart.ptr) % m_DescriptorSize == 0u, "CPU handle pointer is not valid for this descriptor heap.");

		m_FreeList.push(FencedDescriptorHandle(descriptorHandle, syncPoint));
	}

	void DescriptorHeap::SetDebugName() noexcept
	{
		std::string debugName = "Descriptor_Heap_";
		switch (m_Type)
		{
		case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
		{
			debugName += "CBV_SRV_UAV";
			break;
		}
		case D3D12_DESCRIPTOR_HEAP_TYPE_RTV:
		{
			debugName += "RTV";
			break;
		}
		case D3D12_DESCRIPTOR_HEAP_TYPE_DSV:
		{
			debugName += "DSV";
			break;
		}
		case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
		{
			debugName += "SAMPLER";
			break;
		}
		}
		debugName += " [Capacity: ";
		debugName += std::to_string(m_Capacity);
		debugName += "]";
		D3D::SetObjectName(m_pDescriptorHeap, debugName.c_str());
	}
}