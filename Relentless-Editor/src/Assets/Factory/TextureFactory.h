#pragma once
#include <Relentless.h>

#include "FactoryBase.h"

namespace Relentless
{
	class TextureFactory : public FactoryBase
	{
	public:
		void SetCompressionType(ETextureCompressionType compressionType) noexcept;
		void SetGenerateMipmaps(bool enable) noexcept;
		void SetImportAsSRGB(bool enable) noexcept;

		NO_DISCARD bool CanCreateNew() const noexcept override;
		NO_DISCARD bool CanImport(const Path& aPath) const noexcept override;
		virtual Ref<IFactory> Clone() noexcept override;

		virtual FactoryCreateResult CreateNew(const String& aName, const UUID& aUUID = CreateUUID()) noexcept override;

		NO_DISCARD bool DoesSupportAsset(IAsset* aAsset) const noexcept override;

		NO_DISCARD std::vector<String> GetSupportedFileExtensions() const noexcept override;
		NO_DISCARD std::vector<String> GetFormats() const noexcept override;

		void SetImportAsCubemap(bool aShouldConvert) noexcept;
		void SetGraphicsDevice(GraphicsDevice* aGraphicsDevice) noexcept;
		void SetMaxTextureSize(uint32 aMaxSize) noexcept;
		NO_DISCARD bool SupportsFileExtension(const std::string_view aFileExtension) const noexcept override;
		NO_DISCARD bool ShouldShowInNewMenu() const noexcept override;
	protected:
		FactoryResult ImportFromFileImpl(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept override;
	private:
		void Finalize(bool success) noexcept;
		NO_DISCARD bool ImportTexture() noexcept;
	private:
		std::array<String, 9> m_SupportedExensions { ".exr", ".tga", ".jpg", ".jpeg", ".png", ".bmp", ".dds", ".tif", ".hdr" };
		std::array<String, 9> m_SupportedFormats{ "EXR", "TGA", "JPG", "JPEG", "PNG", "BMP", "DDS", "TIFF", "HDR" };

		Path m_SrcPath;

		GraphicsDevice* m_pDevice = nullptr;

		ETextureCompressionType m_CompressionType = ETextureCompressionType::Uncompressed;
		int32 m_MaxSize = -1;
		bool m_GenerateMipmaps = true;
		bool m_IsSRGB = false;
		bool m_ImportAsCubemap = false;
	};
}