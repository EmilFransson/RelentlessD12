#include "RenderUtility.h"
#include "../D3D12Core.h"
#include "../Resources/Texture.h"

namespace Relentless
{
	uint64_t RenderUtility::GetTextureSizeInBytes(const std::shared_ptr<Texture>& pTexture) noexcept
	{
		RLS_ASSERT(pTexture, "Texture is invalid.");

		UINT64 totalBytes{};
		auto desc = pTexture->GetInterface()->GetDesc();

		DXCall_STD(D3D12Core::GetDevice()->GetCopyableFootprints(&desc, 0u, 1u, 0u, nullptr, nullptr, nullptr, &totalBytes));
		return totalBytes;
	}
}