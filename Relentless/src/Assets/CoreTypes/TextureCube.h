#pragma once
#include "Assets/AssetMeta.h"
#include "Assets/IAsset.h"

#include "Core/DLLExport.h"
#include "Core/Ref.h"

#include "Graphics/RHI/Texture.h"

#include <directxtex/DirectXTex.h>

namespace Relentless
{
	class RLS_API TextureCube : public AssetBase<TextureCube>
	{
	public:
		TextureCube(const UUID& aUUID) noexcept;
		TextureCube(const TextureDesc& aTextureDesc, DirectX::ScratchImage&& aImage) noexcept;
		TextureCube(Ref<Texture> aBackingTexture) noexcept;

		void CreateResource() noexcept;

		const DirectX::ScratchImage& GetImage() const noexcept;
		const TextureDesc& GetDesc() const noexcept;
		NO_DISCARD const Ref<Texture>& GetResource() const noexcept;

		static constexpr const UUID& PersistentType()
		{
			static constexpr UUID uid = UUID{ 0xe0e4ea02, 0x1a05, 0x4d85, { 0xb9, 0xb5, 0x5a, 0x84, 0x3a, 0xb0, 0xdc, 0xd } };
			return uid;
		}

		bool SerializeBulk(IArchive& aArchive) noexcept override;
		bool SerializeCore(IArchive& aArchive) noexcept override;
	private:
		TextureDesc m_Desc;
		DirectX::ScratchImage m_ScratchImage;
		Ref<Texture> m_pGPUResource = nullptr;
	};
}
