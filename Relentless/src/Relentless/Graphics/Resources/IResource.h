#pragma once
#include "../DescriptorHeap.h"
namespace Relentless
{
	class IResource
	{
	public:
		[[nodiscard]] constexpr const Microsoft::WRL::ComPtr<ID3D12Resource>& GetInterface() const { return m_pResource; }
		[[nodiscard]] constexpr const DescriptorHandle& GetDescriptorHandle() const { return m_DescriptorHandle; }
		[[nodiscard]] constexpr const D3D12_RESOURCE_STATES& GetCurrentState() const { return m_CurrentState; }
		IResource() noexcept;
		virtual ~IResource() noexcept = default;
	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
		DescriptorHandle m_DescriptorHandle;
		D3D12_RESOURCE_STATES m_CurrentState;
	};
}