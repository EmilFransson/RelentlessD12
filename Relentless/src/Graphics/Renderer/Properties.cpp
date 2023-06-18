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
		}

		RLS_ASSERT(false, "Unsupported texture format encountered.");
		return DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	}
}