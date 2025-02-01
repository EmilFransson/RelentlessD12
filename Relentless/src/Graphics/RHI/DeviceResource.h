#pragma once

#include "RHI.h"

namespace Relentless
{
	class DeviceObject : public RefCounted<DeviceObject>
	{
	public:
		DeviceObject(GraphicsDevice* pParent) noexcept;
		virtual ~DeviceObject() noexcept = default;

		[[nodiscard]] GraphicsDevice* GetParent() const noexcept;
	private:
		GraphicsDevice* m_pParent = nullptr;
	};

	constexpr D3D12_RESOURCE_STATES D3D12_RESOURCE_STATE_UNKNOWN = (D3D12_RESOURCE_STATES)-1;

	class ResourceState
	{
	public:
		void Set(D3D12_RESOURCE_STATES resourceState, uint32_t subResourceIndex = 0u) noexcept
		{
			if (subResourceIndex != D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES)
			{
				D3D12_RESOURCE_STATES current_state = m_ResourceStates[0];
				RLS_ASSERT(subResourceIndex < m_ResourceStates.size(), "Index Out Of Bounds Error");
				if (m_AllUseSameResourceState)
				{
					for (D3D12_RESOURCE_STATES& s : m_ResourceStates)
						s = current_state;
					m_AllUseSameResourceState = false;
				}
				m_ResourceStates[subResourceIndex] = resourceState;
			}
			else
			{
				m_AllUseSameResourceState = true;
				m_ResourceStates[0] = resourceState;
			}
		}

		[[nodiscard]] D3D12_RESOURCE_STATES Get(uint32_t subResourceIndex = 0u) const noexcept
		{
			RLS_ASSERT(subResourceIndex < m_ResourceStates.size(), "Index Out Of Bounds Error");
			return m_ResourceStates[subResourceIndex];
		}
	private:
		std::array<D3D12_RESOURCE_STATES, D3D12_REQ_MIP_LEVELS> m_ResourceStates{};
		bool m_AllUseSameResourceState = true;
	};

	class DeviceResource : public DeviceObject
	{
	public:
		DeviceResource(GraphicsDevice* pParent, ID3D12Resource* pResource) noexcept;
		virtual ~DeviceResource() noexcept override;

		[[nodiscard]] D3D12_GPU_VIRTUAL_ADDRESS GetGpuHandle() const noexcept;
		[[nodiscard]] const std::string& GetName() const noexcept;
		[[nodiscard]] ID3D12Resource* GetResource() const noexcept;
		[[nodiscard]] D3D12_RESOURCE_STATES GetResourceState(uint32_t subResourceIndex = 0u) const noexcept;
		void SetImmediateDelete(bool immediate) noexcept;
		void SetName(const char* pName) noexcept;
		void SetResourceState(D3D12_RESOURCE_STATES resourceState, uint32_t subResourceIndex = 0u) noexcept;
		void SetStateTracking(bool enabled) noexcept;
		[[nodiscard]] bool UsesStateTracking() const noexcept;
	private:
		std::string m_Name;
		ID3D12Resource* m_pResource = nullptr;
		ResourceState m_ResourceState;
		bool m_UsesStateTracking = true;
		bool m_ShouldImmediateDelete = false;
	};

}