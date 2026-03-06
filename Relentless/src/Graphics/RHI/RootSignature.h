#pragma once
#include "DeviceResource.h"

namespace Relentless
{
	class RootSignature : public DeviceObject
	{
	public:
		static constexpr uint8 sMaxNumParameters = 8;

		RootSignature(GraphicsDevice* pParent) noexcept;
		virtual ~RootSignature() noexcept override = default;
		[[nodiscard]] ID3D12RootSignature* GetRootSignature() const noexcept;
		
		void AddRootCBV(uint8 shaderRegister, uint8 space, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept;
		void AddRootConstant(uint8 shaderRegister, uint64 num32BitValues, uint8 space, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept;
		void AddRootSRV(uint8 shaderRegister, uint8 space, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept;
		void AddRootUAV(uint8 shaderRegister, uint8 space, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL) noexcept;
		void AddStaticSampler(uint32 registerSlot, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE wrapMode, D3D12_COMPARISON_FUNC compareFunc = D3D12_COMPARISON_FUNC_ALWAYS) noexcept;
		void AddStaticSampler(uint32 aRegisterSlot, D3D12_FILTER aFilter, D3D12_TEXTURE_ADDRESS_MODE aAddressU, D3D12_TEXTURE_ADDRESS_MODE aAddressV, D3D12_TEXTURE_ADDRESS_MODE aAddressW, D3D12_COMPARISON_FUNC aCompareFunc = D3D12_COMPARISON_FUNC_ALWAYS) noexcept;
		void Finalize(const char* pName) noexcept;
	private:
		Ref<ID3D12RootSignature> m_pRootSignature = nullptr;
		std::array<CD3DX12_ROOT_PARAMETER1, sMaxNumParameters> m_RootParameters{};
		std::vector<D3D12_STATIC_SAMPLER_DESC> m_StaticSamplers;
		uint8 m_NrOfParameters = 0u;
	};
}