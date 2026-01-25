#include "TextureFactory.h"
#include "../../Core/Editor.h"

#include <DirectXTK/WICTextureLoader.h>
#include <DirectXTK/ResourceUploadBatch.h>

namespace Relentless
{
	static void LogHR(HRESULT hr, const std::string& contextualString, const std::filesystem::path& srcFilepath) noexcept
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

	FactoryCreateResult TextureFactory::CreateNew(const String& aName, const UUID& aUUID ) noexcept
	{
		Ref<Texture2D> pNewTexture = new Texture2D(aUUID);
		pNewTexture->SetName(aName);

		return pNewTexture;
	}

	bool TextureFactory::DoesSupportAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Texture2D::StaticType();
	}

	std::vector<String> TextureFactory::GetSupportedFileExtensions() const noexcept
	{
		return std::vector<String>(m_SupportedExensions.begin(), m_SupportedExensions.end());
	}

	std::vector<String> TextureFactory::GetFormats() const noexcept
	{
		return std::vector<String>(m_SupportedFormats.begin(), m_SupportedFormats.end());
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
		if (shouldCompress)
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

		Ref<Texture2D> pNewTexture = new Texture2D(TextureDesc::Create2D(metaData.width, metaData.height, D3D::ConvertFormat(metaData.format), metaData.mipLevels, TextureFlag::ShaderResource), std::move(image));
		pNewTexture->SetName(fileName);

		m_ImportedAsset = AssetManager::RegisterAsset<Texture2D>(pNewTexture);

		return true;
	}

	bool TextureFactory::ShouldShowInNewMenu() const noexcept
	{
		return false;
	}
}