#include "IFactory.h"

namespace Relentless
{
	FactoryCreateResult IFactory::CreateNew(MAYBE_UNUSED const String&, MAYBE_UNUSED const UUID& aUUID) noexcept
	{
		return nullptr;
	}

	const std::vector<FactoryResult>& IFactory::GetAdditionalImportedAssets() const noexcept
	{
		return m_AdditionalImportedAssets;
	}

	String IFactory::GetDefaultNewAssetName() const noexcept
	{
		return "NewAsset";
	}

	String IFactory::GetDisplayName() const noexcept
	{
		return "Unnamed";
	}

	std::vector<String> IFactory::GetSupportedFileExtensions() const noexcept
	{
		return {};
	}

	std::vector<String> IFactory::GetFormats() const noexcept
	{
		return {};
	}

	bool IFactory::SupportsFileExtension(MAYBE_UNUSED const StringView aFileExtension) const noexcept
	{
		return false;
	}

	bool IFactory::ShouldShowInNewMenu() const noexcept
	{
		return CanCreateNew();
	}

	FactoryResult IFactory::ImportFromFileImpl(MAYBE_UNUSED const Path& aPath, MAYBE_UNUSED const Path& aPackagePath, MAYBE_UNUSED const String& aName, MAYBE_UNUSED Ref<FeedbackContext> aFeedbackContext) noexcept
	{
		return AssetHandle::INVALID;
	}
}
