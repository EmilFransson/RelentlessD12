#pragma once
#include "../DescriptorHeap.h"
namespace Relentless
{
	class IResource
	{
	public:
		[[nodiscard]] constexpr const Microsoft::WRL::ComPtr<ID3D12Resource>& GetInterface() const { return m_pResource; }
		[[nodiscard]] constexpr const D3D12_RESOURCE_STATES& GetCurrentState() const { return m_CurrentState; }
		void SetCurrentState(const D3D12_RESOURCE_STATES newState) { m_CurrentState = newState; }
		IResource(const std::string& name = "?") noexcept;
		virtual ~IResource() noexcept = default;
		[[nodiscard]] constexpr const std::string& GetName() const { return m_Name; }
	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> m_pResource;
		std::string m_Name;
		D3D12_RESOURCE_STATES m_CurrentState;
	};
}