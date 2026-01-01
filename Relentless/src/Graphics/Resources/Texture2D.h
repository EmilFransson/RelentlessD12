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

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0xe50a7985, 0xf306, 0x4eb2, { 0x82, 0xbb, 0xf0, 0x8a, 0xc, 0xe2, 0x61, 0x10 }};
			return uid;
		}
	private:
		TextureDesc m_Desc;
		DirectX::ScratchImage m_ScratchImage;
		Ref<Texture> m_pGPUResource = nullptr;
	};
}
