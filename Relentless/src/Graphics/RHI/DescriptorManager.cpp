#include "DescriptorManager.h"
namespace Relentless
{
	DescriptorManager::DescriptorManager(GraphicsDevice* pDevice) noexcept
	{
		m_pRTVDescriptorHeap = new DescriptorHeapEx(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 100'000, false);
		m_pDSVDescriptorHeap = new DescriptorHeapEx(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 100, false);
		m_pShaderBindablesDescriptorHeapNV = new DescriptorHeapEx(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, false);
		m_pShaderBindablesDescriptorHeap = new DescriptorHeapEx(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 100'000, true);
		m_pSamplerDescriptorHeap = new DescriptorHeapEx(pDevice, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE, true);
	}

	const DescriptorHandleEx DescriptorManager::CreateDescriptorHandle(DescriptorHandleTypeEx descriptorHandleType) noexcept
	{
		switch (descriptorHandleType)
		{
		case DescriptorHandleTypeEx::RTV:
		{
			DescriptorHandleEx handle = m_pRTVDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleTypeEx::DSV:
		{
			DescriptorHandleEx handle = m_pDSVDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleTypeEx::SRV_NV:
		{
			DescriptorHandleEx handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleTypeEx::CBV_NV:
		{
			DescriptorHandleEx handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleTypeEx::UAV_NV:
		{
			DescriptorHandleEx handle = m_pShaderBindablesDescriptorHeapNV->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleTypeEx::SRV:
		{
			DescriptorHandleEx handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleTypeEx::CBV:
		{
			DescriptorHandleEx handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		case DescriptorHandleTypeEx::UAV:
		{
			DescriptorHandleEx handle = m_pShaderBindablesDescriptorHeap->AllocateDescriptor();
			handle.Type = descriptorHandleType;
			return handle;
		}
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}
		return DescriptorHandleEx();
	}

	void DescriptorManager::DeferReleaseDescriptorHandle(const DescriptorHandleEx& descriptorHandle, const SyncPoint& syncPoint) noexcept
	{
		switch (descriptorHandle.Type)
		{
		case DescriptorHandleTypeEx::RTV:
			m_pRTVDescriptorHeap->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		case DescriptorHandleTypeEx::DSV:
			m_pDSVDescriptorHeap->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		case DescriptorHandleTypeEx::CBV:
		case DescriptorHandleTypeEx::SRV:
		case DescriptorHandleTypeEx::UAV:
			m_pShaderBindablesDescriptorHeap->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		case DescriptorHandleTypeEx::CBV_NV:
		case DescriptorHandleTypeEx::SRV_NV:
		case DescriptorHandleTypeEx::UAV_NV:
			m_pShaderBindablesDescriptorHeapNV->FreeDescriptor(descriptorHandle, syncPoint);
			break;
		}
	}
}