#include "TextureCube.h"

#include "Core/Application.h"

#include "Graphics/RHI/Device.h"

namespace Relentless
{
	TextureCube::TextureCube(const TextureDesc& aTextureDesc, DirectX::ScratchImage&& aImage) noexcept
		:m_Desc(aTextureDesc),
		m_ScratchImage(std::move(aImage))
	{}

	TextureCube::TextureCube(const UUID& aUUID) noexcept
		: AssetBase<TextureCube>(aUUID)
	{}

	TextureCube::TextureCube(Ref<Texture> aBackingTexture) noexcept
		: m_pGPUResource{aBackingTexture}
	{}

	void TextureCube::CreateResource() noexcept
	{
		GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();
		if (!pDevice)
			return;

		m_pGPUResource = pDevice->CreateTexture(m_Desc, GetName().c_str(), m_ScratchImage);
	}

	const DirectX::ScratchImage& TextureCube::GetImage() const noexcept
	{
		return m_ScratchImage;
	}

	const TextureDesc& TextureCube::GetDesc() const noexcept
	{
		return m_Desc;
	}

	const Ref<Texture>& TextureCube::GetResource() const noexcept
	{
		return m_pGPUResource;
	}

	bool TextureCube::SerializeBulk(IArchive& aArchive) noexcept
	{
		if (aArchive.IsSaving())
		{
			const DirectX::ScratchImage& image = m_ScratchImage;
			const DirectX::TexMetadata& meta = image.GetMetadata();

			aArchive.Process(meta.width);
			aArchive.Process(meta.height);
			aArchive.Process(meta.depth);
			aArchive.Process(meta.arraySize);
			aArchive.Process(meta.mipLevels);
			aArchive.Process(meta.format);
			aArchive.Process(meta.dimension);
			aArchive.Process(meta.miscFlags);
			aArchive.Process(meta.miscFlags2);

			uint64_t pixelByteSize = image.GetPixelsSize();
			aArchive.Process(pixelByteSize);

			if (pixelByteSize == 0)
				return aArchive.IsValid();

			uint8* pixels = image.GetPixels();
			if (!pixels)
				return false;

			if (!aArchive.ProcessRaw(pixels, static_cast<size_t>(pixelByteSize)))
				return false;
		}
		else //Loading
		{
			DirectX::TexMetadata meta{};

			aArchive.Process(meta.width);
			aArchive.Process(meta.height);
			aArchive.Process(meta.depth);
			aArchive.Process(meta.arraySize);
			aArchive.Process(meta.mipLevels);
			aArchive.Process(meta.format);
			aArchive.Process(meta.dimension);
			aArchive.Process(meta.miscFlags);
			aArchive.Process(meta.miscFlags2);

			uint64_t pixelByteSize = 0;
			aArchive.Process(pixelByteSize);

			if (pixelByteSize == 0)
			{
				m_ScratchImage.Release();
				return aArchive.IsValid();
			}

			if (FAILED(m_ScratchImage.Initialize(meta)))
				return false;

			uint8_t* pixels = m_ScratchImage.GetPixels();
			if (!pixels)
				return false;

			if (!aArchive.ProcessRaw(pixels, static_cast<size_t>(pixelByteSize)))
				return false;
		}

		return aArchive.IsValid();
	}

	bool TextureCube::SerializeCore(IArchive& aArchive) noexcept
	{
		return aArchive.Process(m_Desc);
	}
}