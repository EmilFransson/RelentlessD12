#include "Properties.h"
namespace Relentless
{
	DXGI_FORMAT RLSTextureFormatToDXGITextureFormat(TextureFormat format) noexcept
	{
		switch (format)
		{
		case TextureFormat::RGBA32F:
			return DXGI_FORMAT::DXGI_FORMAT_R32G32B32A32_FLOAT;
			break;
		case TextureFormat::R32UINT:
			return DXGI_FORMAT::DXGI_FORMAT_R32_UINT;
			break;
		case TextureFormat::R32TYPELESS:
			return DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
			break;
		case TextureFormat::Depth:
			return DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;
			break;
		}

		RLS_ASSERT(false, "Unsupported texture format encountered.");
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	}

	D3D12_ROOT_PARAMETER_TYPE HLSLParameterTypeToD3D12RootParameterType(D3D_SHADER_INPUT_TYPE inputType) noexcept
	{
		switch (inputType)
		{
		case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:
			return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			break;
		case D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED:
			return D3D12_ROOT_PARAMETER_TYPE_SRV;
			break;
		}

		RLS_ASSERT(false, "Unknown shader type encountered.");
		return D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	}
}