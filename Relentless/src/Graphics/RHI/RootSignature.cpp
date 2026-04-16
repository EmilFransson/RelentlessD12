#include "RootSignature.h"
#include "Device.h"
#include "D3D.h"

namespace Relentless
{
	RootSignature::RootSignature(GraphicsDevice* pParent) noexcept
		:DeviceObject(pParent)
	{}

	ID3D12RootSignature* RootSignature::GetRootSignature() const noexcept
	{
		return m_pRootSignature;
	}

	void RootSignature::AddRootCBV(uint8 shaderRegister, uint8 space, D3D12_SHADER_VISIBILITY visibility) noexcept
	{
		CD3DX12_ROOT_PARAMETER1& rootParam = m_RootParameters[m_NrOfParameters++];
		rootParam.InitAsConstantBufferView(shaderRegister, space, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, visibility);
	}

	void RootSignature::AddRootConstant(uint8 shaderRegister, uint64 num32BitValues, uint8 space, D3D12_SHADER_VISIBILITY visibility) noexcept
	{
		CD3DX12_ROOT_PARAMETER1& rootParam = m_RootParameters[m_NrOfParameters++];
		rootParam.InitAsConstants(static_cast<UINT>(num32BitValues), shaderRegister, space, visibility);
	}

	void RootSignature::AddRootSRV(uint8 shaderRegister, uint8 space, D3D12_SHADER_VISIBILITY visibility) noexcept
	{
		CD3DX12_ROOT_PARAMETER1& rootParam = m_RootParameters[m_NrOfParameters++];
		rootParam.InitAsShaderResourceView(shaderRegister, space, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, visibility);
	}

	void RootSignature::AddRootUAV(uint8 shaderRegister, uint8 space, D3D12_SHADER_VISIBILITY visibility) noexcept
	{
		CD3DX12_ROOT_PARAMETER1& rootParam = m_RootParameters[m_NrOfParameters++];
		rootParam.InitAsUnorderedAccessView(shaderRegister, space, D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE, visibility);
	}

	void RootSignature::AddStaticSampler(uint32 registerSlot, D3D12_FILTER filter, D3D12_TEXTURE_ADDRESS_MODE wrapMode, D3D12_COMPARISON_FUNC compareFunc) noexcept
	{
		D3D12_STATIC_SAMPLER_DESC desc{};
		desc.AddressU = wrapMode;
		desc.AddressV = wrapMode;
		desc.AddressW = wrapMode;
		desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		desc.ComparisonFunc = compareFunc;
		desc.Filter = filter;
		desc.MaxAnisotropy = 16;
		desc.MaxLOD = FLT_MAX;
		desc.MinLOD = 0.0f;
		desc.RegisterSpace = 1;
		desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		desc.ShaderRegister = registerSlot;
		desc.MipLODBias = 0.0f;
		m_StaticSamplers.push_back(desc);
	}

	void RootSignature::AddStaticSampler(uint32 aRegisterSlot, D3D12_FILTER aFilter, D3D12_TEXTURE_ADDRESS_MODE aAddressU, D3D12_TEXTURE_ADDRESS_MODE aAddressV, D3D12_TEXTURE_ADDRESS_MODE aAddressW, D3D12_COMPARISON_FUNC aCompareFunc /*= D3D12_COMPARISON_FUNC_ALWAYS*/) noexcept
	{
		D3D12_STATIC_SAMPLER_DESC desc{};
		desc.AddressU = aAddressU;
		desc.AddressV = aAddressV;
		desc.AddressW = aAddressW;
		desc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		desc.ComparisonFunc = aCompareFunc;
		desc.Filter = aFilter;
		desc.MaxAnisotropy = 16;
		desc.MaxLOD = FLT_MAX;
		desc.MinLOD = 0.0f;
		desc.RegisterSpace = 1;
		desc.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
		desc.ShaderRegister = aRegisterSlot;
		desc.MipLODBias = 0.0f;
		m_StaticSamplers.push_back(desc);
	}

	void RootSignature::Finalize(const char* pName) noexcept
	{
		D3D12_ROOT_SIGNATURE_FLAGS flags =
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS
			| D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;

		int staticSamplerRegisterSlot = 0;
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_BORDER);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_MIN_MAG_MIP_LINEAR, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_MIN_MAG_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_BORDER);

		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_GREATER);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_CLAMP, D3D12_COMPARISON_FUNC_GREATER);
		AddStaticSampler(staticSamplerRegisterSlot++, D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_COMPARISON_FUNC_GREATER);

		for (size_t i = 0; i < m_NrOfParameters; ++i)
		{
			const CD3DX12_ROOT_PARAMETER1& rootParameter = m_RootParameters[i];
			switch (rootParameter.ShaderVisibility)
			{
			case D3D12_SHADER_VISIBILITY_VERTEX:		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;			break;
			case D3D12_SHADER_VISIBILITY_GEOMETRY:		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;		break;
			case D3D12_SHADER_VISIBILITY_PIXEL:			flags = flags & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;			break;
			case D3D12_SHADER_VISIBILITY_HULL:			flags = flags & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;			break;
			case D3D12_SHADER_VISIBILITY_DOMAIN:		flags = flags & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;			break;
			case D3D12_SHADER_VISIBILITY_MESH:			flags = flags & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;			break;
			case D3D12_SHADER_VISIBILITY_AMPLIFICATION:	flags = flags & ~D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS;	break;
			case D3D12_SHADER_VISIBILITY_ALL:			flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;												break;
			default:									RLS_ASSERT(false, "Unreachable");													break;
			}
		}

		flags |= D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;
		flags |= D3D12_ROOT_SIGNATURE_FLAG_SAMPLER_HEAP_DIRECTLY_INDEXED;

		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC desc = {};
		desc.Init_1_1(m_NrOfParameters, m_RootParameters.data(), (uint32)m_StaticSamplers.size(), m_StaticSamplers.data(), flags);

		Ref<ID3DBlob> pDataBlob = nullptr;
		Ref<ID3DBlob> pErrorBlob = nullptr;
		D3D12SerializeVersionedRootSignature(&desc, pDataBlob.GetAddressOf(), pErrorBlob.GetAddressOf());
		if (pErrorBlob)
		{
			RLS_CORE_ERROR("RootSignature serialization error: {0}", (char*)pErrorBlob->GetBufferPointer());
			return;
		}
		
		VERIFY_HR_EX(GetParent()->GetDevice()->CreateRootSignature(0u, pDataBlob->GetBufferPointer(), pDataBlob->GetBufferSize(), IID_PPV_ARGS(m_pRootSignature.ReleaseAndGetAddressOf())), GetParent()->GetDevice());
		D3D::SetObjectName(m_pRootSignature, pName);
	}
}