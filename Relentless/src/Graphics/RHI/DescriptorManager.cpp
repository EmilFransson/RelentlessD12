#include "DescriptorManager.h"
namespace Relentless
{
	DescriptorManager::DescriptorManager(GraphicsDevice* pDevice) noexcept
	{
		m_pRTVDescriptorHeap = new DescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100'000, false);
		m_pDSVDescriptorHeap = new DescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 100, false);
		m_pShaderBindablesDescriptorHeapNV = new DescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, false);
		m_pShaderBindablesDescriptorHeap = new DescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, true);
		m_pSamplerDescriptorHeap = new DescriptorHeap(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE, true);
	}

	const DescriptorHandle DescriptorManager::CreateDescriptorHandle(DescriptorHandleType descriptorHandleType) noexcept
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
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::CBV_NV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::UAV_NV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::SRV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::CBV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleType::UAV:
		{
			DescriptorHandle handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}
		return DescriptorHandle();
	}

	const std::vector<DescriptorHandle> DescriptorManager::CreateDescriptorHandleBlock(DescriptorHandleType descriptorHandleType, uint32 blockSize) noexcept
	{
		std::vector<DescriptorHandle> handles;

		switch (descriptorHandleType)
		{
		case DescriptorHandleType::RTV:
		{
			handles = m_pRTVDescriptorHeap->AllocateDescriptorBlock(blockSize);
			break;
		}
		case DescriptorHandleType::DSV:
		{
			handles = m_pDSVDescriptorHeap->AllocateDescriptorBlock(blockSize);
			break;
		}
		case DescriptorHandleType::SRV_NV:
		{
			handles = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptorBlock(blockSize);
			break;
		}
		case DescriptorHandleType::CBV_NV:
		{
			handles = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptorBlock(blockSize);
			break;
		}
		case DescriptorHandleType::UAV_NV:
		{
			handles = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptorBlock(blockSize);
			break;
		}
		case DescriptorHandleType::SRV:
		{
			handles = m_pShaderBindablesDescriptorHeap->AllocateDescriptorBlock(blockSize);
			break;
		}
		case DescriptorHandleType::CBV:
		{
			handles = m_pShaderBindablesDescriptorHeap->AllocateDescriptorBlock(blockSize);
			break;
		}
		case DescriptorHandleType::UAV:
		{
			handles = m_pShaderBindablesDescriptorHeap->AllocateDescriptorBlock(blockSize);
			break;
		}
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}

		std::for_each(handles.begin(), handles.end(), [&descriptorHandleType](DescriptorHandle& handle)
			{
				handle.Type = descriptorHandleType;
			});

		return handles;
	}

	void DescriptorManager::DeferReleaseDescriptorHandle(const DescriptorHandle& descriptorHandle, const SyncPoint& syncPoint) noexcept
	{
		switch (descriptorHandle.Type)
		{
		case DescriptorHandleType::RTV:
			m_pRTVDescriptorHeap->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		case DescriptorHandleType::DSV:
			m_pDSVDescriptorHeap->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		case DescriptorHandleType::CBV:
		case DescriptorHandleType::SRV:
		case DescriptorHandleType::UAV:
			m_pShaderBindablesDescriptorHeap->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		case DescriptorHandleType::CBV_NV:
		case DescriptorHandleType::SRV_NV:
		case DescriptorHandleType::UAV_NV:
			m_pShaderBindablesDescriptorHeapNV->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		default:
		{
			RLS_ASSERT(false, "Unreachable.");
			break;
		}
		}
	}
}