#include "Texture2D.h"
#include "Core/Application.h"
#include "Graphics/RHI/Device.h"

namespace Relentless
{
	Texture2D::Texture2D(const TextureDesc& aTextureDesc, DirectX::ScratchImage&& aImage) noexcept
		:m_Desc(aTextureDesc),
		 m_ScratchImage(std::move(aImage))
	{
	}

	Texture2D::Texture2D(const UUID& aUUID) noexcept
		: AssetBase<Texture2D>(aUUID)
	{
	}

	void Texture2D::CreateResource() noexcept
	{
		GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();
		if (!pDevice)
			return;

		m_pGPUResource = pDevice->CreateTexture(m_Desc, GetName().c_str(), m_ScratchImage);
	}

	const DirectX::ScratchImage& Texture2D::GetImage() const noexcept
	{
		return m_ScratchImage;
	}

	const TextureDesc& Texture2D::GetDesc() const noexcept
	{
		return m_Desc;
	}

	const Ref<Texture>& Texture2D::GetResource() const noexcept
	{
		return m_pGPUResource;
	}

	bool Texture2D::SerializeBulk(IArchive& aArchive) noexcept
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

	bool Texture2D::SerializeCore(IArchive& aArchive) noexcept
	{
		return aArchive.Process(m_Desc);
	}

}