#pragma once

#include "Assets/AssetMeta.h"
#include "Assets/ImportSettings.h"

#include "Core/DLLExport.h"
#include "Core/IAsset.h"

#include "FeedbackContext.h"

namespace Relentless
{
	using FactoryResult = std::expected<AssetHandle, String>;
	using FactoryCreateResult = std::expected<Ref<IAsset>, String>;

	class RLS_API IFactory : public RefCounted<IFactory>
	{
	public:
		virtual ~IFactory() = default;

		NO_DISCARD virtual bool CanCreateNew() const noexcept = 0;
		NO_DISCARD virtual bool CanImport(const Path& aPath) const noexcept = 0;
		NO_DISCARD virtual Ref<IFactory> Clone() noexcept = 0;
		virtual FactoryCreateResult CreateNew(const String&, const UUID& aUUID = CreateUUID()) noexcept;

		NO_DISCARD virtual bool DoesSupportAsset(IAsset* aAsset) const noexcept = 0;

		NO_DISCARD const std::vector<FactoryResult>& GetAdditionalImportedAssets() const noexcept;
		NO_DISCARD virtual String GetDefaultNewAssetName() const noexcept;
		NO_DISCARD virtual String GetDisplayName() const noexcept;
		NO_DISCARD virtual std::vector<String> GetSupportedFileExtensions() const noexcept;
		NO_DISCARD virtual std::vector<String> GetFormats() const noexcept;

		virtual FactoryResult ImportFromFile(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept = 0;

		NO_DISCARD virtual bool SupportsFileExtension(const StringView aFileExtension) const noexcept;
		NO_DISCARD virtual bool ShouldShowInNewMenu() const noexcept;
	protected:
		virtual FactoryResult ImportFromFileImpl(const Path& aPath, const Path& aPackagePath, const String& aName, Ref<FeedbackContext> aFeedbackContext = nullptr) noexcept;
	protected:
		std::vector<FactoryResult> m_AdditionalImportedAssets;
		FactoryResult m_ImportedAsset;
	};
}