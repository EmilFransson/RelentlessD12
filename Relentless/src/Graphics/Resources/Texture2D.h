#pragma once
#include "Assets/AssetMeta.h"
#include "Core/IAsset.h"
#include "Core/Ref.h"

#include "Graphics/RHI/Texture.h"

#include "../../../vendor/includes/directxtex/DirectXTex.h"

namespace Relentless
{
	class Texture2D : public AssetBase<Texture2D>
	{
	public:
		Texture2D(const TextureDesc& aTextureDesc, DirectX::ScratchImage&& aImage) noexcept;

		void CreateResource() noexcept;
		const DirectX::ScratchImage& GetImage() const noexcept;
		const TextureDesc& GetDesc() const noexcept;
		NO_DISCARD const Ref<Texture>& GetResource() const noexcept;
	private:
		TextureDesc m_Desc;
		DirectX::ScratchImage m_ScratchImage;
		Ref<Texture> m_pGPUResource = nullptr;
	};
}
