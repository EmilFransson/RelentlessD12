#include "DeviceResource.h"
#include "Device.h"

namespace Relentless
{
	DeviceResource::DeviceResource(GraphicsDevice* pParent, ID3D12Resource* pResource) noexcept
		:
		DeviceObject(pParent),
		m_pResource(pResource)
	{}
	
	DeviceResource::~DeviceResource() noexcept
	{
		if (m_pResource)
		{
			if (m_ShouldImmediateDelete)
				m_pResource->Release();
			else
				GetParent()->DeferReleaseObject(m_pResource);

			m_pResource = nullptr;
		}
	}

	D3D12_GPU_VIRTUAL_ADDRESS DeviceResource::GetGpuHandle() const noexcept
	{
		return m_pResource->GetGPUVirtualAddress();
	}

	const std::string& DeviceResource::GetName() const noexcept
	{
		return m_Name;
	}

	ID3D12Resource* DeviceResource::GetResource() const noexcept
	{
		return m_pResource;
	}

	D3D12_RESOURCE_STATES DeviceResource::GetResourceState(uint32_t subResourceIndex /*= 0u*/) const noexcept
	{
		return m_ResourceState.Get(subResourceIndex);
	}

	void DeviceResource::SetImmediateDelete(bool immediate) noexcept
	{
		m_ShouldImmediateDelete = immediate;
	}

	void DeviceResource::SetName(const char* pName) noexcept
	{
		m_Name = pName;
	}

	void DeviceResource::SetResourceState(D3D12_RESOURCE_STATES resourceState, uint32_t subResourceIndex /*= 0u*/) noexcept
	{
		m_ResourceState.Set(resourceState, subResourceIndex);
	}

	void DeviceResource::SetStateTracking(bool enabled) noexcept
	{
		m_UsesStateTracking = enabled;
	}

	bool DeviceResource::UsesStateTracking() const noexcept
	{
		return m_UsesStateTracking;
	}

}