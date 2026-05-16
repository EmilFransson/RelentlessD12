#include "TextureFactory.h"
#include "../../Core/Editor.h"

#include <DirectXTK/WICTextureLoader.h>
#include <DirectXTK/ResourceUploadBatch.h>

namespace Relentless
{
	static void LogHR(HRESULT hr, MAYBE_UNUSED const std::string& contextualString, MAYBE_UNUSED const std::filesystem::path& srcFilepath) noexcept
	{
		if (hr != S_OK)
		{
			const _com_error error(hr);
			RLS_CORE_ERROR("[TextureFactory]: Failed to {0} file with path '{1}'; operation failed with message '{2}'", contextualString.c_str(), srcFilepath.string().c_str(), error.ErrorMessage());
		}
	}

	static DXGI_FORMAT GetCompressedDXGITextureFormat(ETextureCompressionType compressionType, bool srgb) noexcept
	{
		DXGI_FORMAT compressedFormat = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
		switch (compressionType)
		{
		case ETextureCompressionType::BC5:
		{
			compressedFormat = DXGI_FORMAT::DXGI_FORMAT_BC5_UNORM;
			break;
		}
		case ETextureCompressionType::BC6_HDR_Unsigned:
		{
			compressedFormat = DXGI_FORMAT::DXGI_FORMAT_BC6H_UF16;
			break;
		}
		case ETextureCompressionType::BC7:
		case ETextureCompressionType::BC7_Quick:
		{
			compressedFormat = srgb ? DXGI_FORMAT::DXGI_FORMAT_BC7_UNORM_SRGB : DXGI_FORMAT_BC7_UNORM;
			break;
		}
		default:
			RLS_ASSERT(false, "Unreachable.")
				return compressedFormat;
		}

		return compressedFormat;
	}

	void TextureFactory::SetCompressionType(ETextureCompressionType compressionType) noexcept
	{
		m_CompressionType = compressionType;
	}

	void TextureFactory::SetGenerateMipmaps(bool enable) noexcept
	{
		m_GenerateMipmaps = enable;
	}

	void TextureFactory::SetImportAsSRGB(bool enable) noexcept
	{
		m_IsSRGB = enable;
	}

	bool TextureFactory::CanCreateNew() const noexcept
	{
		return true;
	}

	bool TextureFactory::CanImport(const Path& aPath) const noexcept
	{
		const String extension = FilepathUtils::ExtractExtension(aPath);
		return std::ranges::any_of(m_SupportedExensions, [&extension](const String& aExtension) { return aExtension == extension; });
	}

	Ref<IFactory> TextureFactory::Clone() noexcept
	{
		return new TextureFactory();
	}

	FactoryCreateResult TextureFactory::CreateNew(const TypeIndex& aType, const String& aName, const UUID& aUUID ) noexcept
	{
		if (aType == Texture2D::StaticType())
		{
			Ref<Texture2D> pNewTexture = RLS_NEW Texture2D(aUUID);
			pNewTexture->SetName(aName);
			return pNewTexture;
		}
		else if (aType == TextureCube::StaticType())
		{
			Ref<TextureCube> pNewTexture = RLS_NEW TextureCube(aUUID);
			pNewTexture->SetName(aName);
			return pNewTexture;
		}
		else
		{
			RLS_ASSERT(false, "[TextureFactory::CreateNew]: Texture Factory does not support given asset type.");
			return {};
		}
	}

	bool TextureFactory::DoesSupportAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Texture2D::StaticType() || aAsset->GetStaticType() == TextureCube::StaticType();
	}

	std::vector<String> TextureFactory::GetSupportedFileExtensions() const noexcept
	{
		return std::vector<String>(m_SupportedExensions.begin(), m_SupportedExensions.end());
	}

	std::vector<String> TextureFactory::GetFormats() const noexcept
	{
		return std::vector<String>(m_SupportedFormats.begin(), m_SupportedFormats.end());
	}

	void TextureFactory::SetImportAsCubemap(bool aImportAsCubemap) noexcept
	{
		m_ImportAsCubemap = aImportAsCubemap;
	}

	FactoryResult TextureFactory::ImportFromFileImpl(const Path& aPath, MAYBE_UNUSED const Path& aPackagePath, MAYBE_UNUSED const String& aName, MAYBE_UNUSED Ref<FeedbackContext> aFeedbackContext) noexcept
	{
		if (!File::Exists(aPath))
			return std::unexpected{"File does not exist."};

		m_SrcPath = aPath;

		if (!ImportTexture())
			return std::unexpected{ "Failed to import texture." };

		return m_ImportedAsset;
	}

	void TextureFactory::SetGraphicsDevice(GraphicsDevice* aGraphicsDevice) noexcept
	{
		m_pDevice = aGraphicsDevice;
	}

	void TextureFactory::SetMaxTextureSize(uint32 aMaxSize) noexcept
	{
		m_MaxSize = static_cast<int>(aMaxSize);
	}

	bool TextureFactory::SupportsFileExtension(const std::string_view aFileExtension) const noexcept
	{
		return std::ranges::any_of(m_SupportedExensions, [&aFileExtension](const String& aExtension) { return aExtension == aFileExtension; });
	}

	void TextureFactory::Finalize(bool /*success*/) noexcept
	{
		//OnDone(m_ImportedTextureAsset, success);
	}

	bool TextureFactory::ImportTexture() noexcept
	{
		DirectX::ScratchImage image;
		HRESULT result = S_OK;

		const String extension = FilepathUtils::ExtractExtension(m_SrcPath);
		if (extension == ".tga")
			result = LoadFromTGAFile(m_SrcPath.c_str(), nullptr, image);
		else if (extension == ".jpg" || extension == ".jpeg" || extension == ".png" || extension == ".tif" || extension == ".tiff")
		{
			DirectX::WIC_FLAGS importFlags = DirectX::WIC_FLAGS::WIC_FLAGS_NONE;
			if (m_IsSRGB)
				importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_SRGB;
			else
				importFlags |= DirectX::WIC_FLAGS::WIC_FLAGS_FORCE_RGB;

			result = LoadFromWICFile(m_SrcPath.c_str(), importFlags, nullptr, image);
		}
		else if (extension == ".hdr" || extension == ".exr")
			result = LoadFromHDRFile(m_SrcPath.c_str(), nullptr, image);
		else if (extension == ".dds")
			result = LoadFromDDSFile(m_SrcPath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
		else
		{
			RLS_CORE_ERROR("[TextureFactory::ImportTexture]: Failed to import texture file with path '{0}'; file type is not supported.", m_SrcPath.string().c_str());
			return false;
		}

		if (result != S_OK)
		{
			LogHR(result, "import", m_SrcPath);
			return false;
		}

		if (image.GetMetadata().format == DXGI_FORMAT_B8G8R8A8_UNORM && m_IsSRGB)
			image.OverrideFormat(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
		else if (image.GetMetadata().format == DXGI_FORMAT_R8G8B8A8_UNORM && m_IsSRGB)
			image.OverrideFormat(DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);

		if (m_GenerateMipmaps)
		{
			DirectX::ScratchImage mipChain;
			const HRESULT hr = GenerateMipMaps(image.GetImages()[0], DirectX::TEX_FILTER_DEFAULT, 0u, mipChain);
			if (hr != S_OK)
				LogHR(hr, "generate mipmaps", m_SrcPath);
			else
				image = std::move(mipChain);
		}

		const bool shouldCompress = m_CompressionType != ETextureCompressionType::Uncompressed;
		if (shouldCompress && !m_ImportAsCubemap)
		{
			DirectX::TEX_COMPRESS_FLAGS compressFlags = DirectX::TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL;
			if (m_CompressionType == ETextureCompressionType::BC7_Quick)
				compressFlags |= DirectX::TEX_COMPRESS_BC7_QUICK;

			DirectX::ScratchImage compressedImage;
			const HRESULT hr = Compress(image.GetImages(), image.GetImageCount(), image.GetMetadata(), GetCompressedDXGITextureFormat(m_CompressionType, m_IsSRGB), compressFlags, DirectX::TEX_THRESHOLD_DEFAULT, compressedImage);
			if (hr != S_OK)
				LogHR(hr, "compress", m_SrcPath);
			else
				image = std::move(compressedImage);
		}

		auto& metaData = image.GetMetadata();
		const std::string fileName = FilepathUtils::ExtractFilename(m_SrcPath);

		if (m_ImportAsCubemap)
		{
			RenderModule& renderModule = ModuleManager::LoadModuleChecked<RenderModule>();
			GraphicsDevice* pDevice = Application::Get().GetGraphicsDevice();

			EquirectangularToCubemapSpecification spec;
			spec.CubeFaceDimension = (m_MaxSize == -1) ? Math::Max(metaData.width, metaData.height) : m_MaxSize;
			spec.EquirectangularTexture = pDevice->CreateTexture(TextureDesc::Create2D(metaData.width, metaData.height, D3D::ConvertFormat(metaData.format), metaData.mipLevels, TextureFlag::ShaderResource), fileName.c_str(), image); //pNewTexture->GetResource();

			Ref<Texture> pOutCubemap = nullptr;
			RenderJobHandle renderJobHandle = renderModule.GetRenderBakeService()->RequestEquirectangularToCubemapConversion(spec, pOutCubemap);
			renderJobHandle.Wait();
			
			//Read back content:
			const uint32 faceCount = 6;                     
			const uint32 mipLevels = pOutCubemap->GetMipLevels();
			const uint32 totalSubresources = faceCount * mipLevels;
			D3D12_RESOURCE_DESC resourceDesc = pOutCubemap->GetResource()->GetDesc();

			std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> footprints(totalSubresources);
			uint64 totalSize = 0;

			for (uint32 subresource = 0; subresource < totalSubresources; subresource++)
			{
				uint64 subresourceSize = 0;
				pDevice->GetDevice()->GetCopyableFootprints(&resourceDesc, subresource, 1, Math::AlignUp(totalSize, (uint64)D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT), &footprints[subresource], nullptr, nullptr, &subresourceSize);
				totalSize = footprints[subresource].Offset + subresourceSize;
			}

			Ref<Buffer> pReadBackBuffer = pDevice->CreateBuffer(BufferDesc::CreateReadback(totalSize), "Cubemap Readback Buffer");

			CommandContext* pContext = pDevice->AllocateCommandContext();
			pContext->InsertResourceBarrier(pOutCubemap, D3D12_RESOURCE_STATE_COPY_SOURCE);

			for (uint32 face = 0; face < faceCount; face++)
			{
				for (uint32 mip = 0; mip < mipLevels; mip++)
				{
					const uint32 subresource = mip + face * mipLevels;
					const uint32 mipWidth = Math::Max(1u, pOutCubemap->GetWidth() >> mip);
					const uint32 mipHeight = Math::Max(1u, pOutCubemap->GetHeight() >> mip);
					D3D12_BOX sourceRegion{ 0, 0, 0, mipWidth, mipHeight, 1 };

					pContext->CopyTexture(pOutCubemap, pReadBackBuffer, sourceRegion, subresource, (uint32)footprints[subresource].Offset);
				}
			}

			SyncPoint fence = pContext->Execute();
			fence.Wait();

			DirectX::TexMetadata meta{};
			meta.width = pOutCubemap->GetWidth();
			meta.height = pOutCubemap->GetHeight();
			meta.depth = 1;
			meta.arraySize = faceCount;
			meta.mipLevels = mipLevels;
			meta.format = D3D::ConvertFormat(pOutCubemap->GetFormat());
			meta.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;
			meta.miscFlags = DirectX::TEX_MISC_TEXTURECUBE;

			DirectX::ScratchImage result;
			result.Initialize(meta);

			char* pBase = (char*)pReadBackBuffer->GetMappedData();

			for (uint32 face = 0; face < faceCount; face++)
			{
				for (uint32 mip = 0; mip < mipLevels; mip++)
				{
					const uint32 subresource = mip + face * mipLevels;
					const uint32 mipHeight = Math::Max(1u, pOutCubemap->GetHeight() >> mip);

					const DirectX::Image* pImage = result.GetImage(mip, face, 0);
					char* pSubresourceData = pBase + footprints[subresource].Offset;

					for (uint32 row = 0; row < mipHeight; row++)
					{
						memcpy(pImage->pixels + row * pImage->rowPitch, pSubresourceData + row * footprints[subresource].Footprint.RowPitch, pImage->rowPitch);                                   
					}
				}
			}

			if (shouldCompress)
			{
				DirectX::ScratchImage compressedCubemap;

				const HRESULT hr = Compress(result.GetImages(), result.GetImageCount(), result.GetMetadata(), DXGI_FORMAT_BC6H_UF16, DirectX::TEX_COMPRESS_FLAGS::TEX_COMPRESS_PARALLEL, DirectX::TEX_THRESHOLD_DEFAULT, compressedCubemap);

				if (FAILED(hr))
					LogHR(hr, "compress cubemap", m_SrcPath);
				else
				{
					meta = compressedCubemap.GetMetadata();
					result = std::move(compressedCubemap);
				}
			}
			

			Ref<TextureCube> pNewCubeMap = RLS_NEW TextureCube(pOutCubemap, TextureDesc::CreateCube(meta.width, meta.height, D3D::ConvertFormat(meta.format), meta.mipLevels, TextureFlag::ShaderResource), std::move(result));
			pNewCubeMap->SetName(fileName);

			m_ImportedAsset = AssetManager::RegisterAsset<TextureCube>(pNewCubeMap);
		}
		else
		{
			Ref<Texture2D> pNewTexture = new Texture2D(TextureDesc::Create2D(metaData.width, metaData.height, D3D::ConvertFormat(metaData.format), metaData.mipLevels, TextureFlag::ShaderResource), std::move(image));
			pNewTexture->SetName(fileName);

			m_ImportedAsset = AssetManager::RegisterAsset<Texture2D>(pNewTexture);
		}

		return true;
	}

	bool TextureFactory::ShouldShowInNewMenu() const noexcept
	{
		return false;
	}
}