#include "IAssetDefinition.h"

namespace Relentless
{

	std::vector<String> IAssetDefinition::GetAssetCategories() const noexcept
	{
		return { "Misc" };
	}

	String IAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Asset";
	}

	Color IAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::White;
	}

	TypeIndex IAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return INVALID_TYPE::StaticType();
	}

	Ref<ThumbnailInfo> IAssetDefinition::GetThumbnailInfo(MAYBE_UNUSED const AssetData& aAssetData) const noexcept
	{
		return nullptr;
	}

	bool IAssetDefinition::OpenAssets(MAYBE_UNUSED const std::vector<AssetHandle>& someAssets) noexcept
	{
		return false;
	}

	bool IAssetDefinition::RequestGenerateThumbnail(MAYBE_UNUSED const AssetData& aAssetData, MAYBE_UNUSED const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept
	{
		return false;
	}

	bool IAssetDefinition::SupportsCreateNew() const noexcept
	{
		return false;
	}

	bool IAssetDefinition::SupportsAsset(MAYBE_UNUSED IAsset* aAsset) const noexcept
	{
		return false;
	}

	bool IAssetDefinition::SupportsAsset(MAYBE_UNUSED AssetData* aAssetData) const noexcept
	{
		return false;
	}

	bool IAssetDefinition::SupportsAsset(MAYBE_UNUSED const AssetHandle& aAssetHandle) const noexcept
	{
		return false;
	}

}