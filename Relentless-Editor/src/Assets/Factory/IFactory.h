#pragma once

#include "../ImportSettings.h"
#include "FeedbackContext.h"

namespace Relentless
{
	enum class EExtensionType : uint8_t { UNKNOWN = 0, TGA, JPG, JPEG, PNG, BMP, DDS, TIFF, HDR, EXR, FBX, OBJ, GLTF };

	using FactoryResult = std::expected<AssetHandle, String>;

	class IFactory : public RefCounted<IFactory>
	{
	public:
		virtual ~IFactory() = default;

		virtual NO_DISCARD bool CanCreateNew() const noexcept = 0;
		virtual NO_DISCARD bool CanImport(const Path& aPath) const noexcept = 0;
		virtual NO_DISCARD Ref<IFactory> Clone() noexcept = 0;
		virtual FactoryResult CreateNew(const String& aName, const Path& aPackagePath) noexcept { return AssetHandle::INVALID; }

		virtual NO_DISCARD bool DoesSupportAsset(IAsset* aAsset) const noexcept = 0;

		NO_DISCARD const std::vector<FactoryResult>& GetAdditionalImportedAssets() const noexcept { return m_AdditionalImportedAssets; }
		virtual NO_DISCARD String GetDefaultNewAssetName() const noexcept { return "NewAsset"; }
		virtual NO_DISCARD String GetDisplayName() const noexcept { return "Unnamed"; }
		virtual NO_DISCARD std::vector<String> GetSupportedFileExtensions() const noexcept { return {}; }
		virtual NO_DISCARD std::vector<String> GetFormats() const noexcept { return {}; }

		const FactoryResult& ImportFromFile(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept;

		virtual NO_DISCARD bool SupportsFileExtension(const std::string_view aFileExtension) const noexcept { return false; }
	protected:
		virtual const FactoryResult& ImportFromFileImpl(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept { return AssetHandle::INVALID; }
	protected:
		std::vector<FactoryResult> m_AdditionalImportedAssets;
		FactoryResult m_ImportedAsset;
	};
}