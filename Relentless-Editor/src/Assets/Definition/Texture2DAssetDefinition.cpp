#include "Texture2DAssetDefinition.h"

namespace Relentless
{
	Color Texture2DAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::Red;
	}

	String Texture2DAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Texture2D";
	}

	TypeIndex Texture2DAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return Texture2D::StaticType();
	}

	bool Texture2DAssetDefinition::RequestGenerateThumbnail(const AssetData& aAssetData, const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept
	{
		if (AssetHandle handle = AssetManager::LoadAsset(aAssetData); handle.IsValid())
		{
			Ref<Texture2D> pTexture2D = AssetManager::Get<Texture2D>(handle);
			aOnThumbnailGeneratedCallback(pTexture2D);
			return true;
		}

		return IAssetDefinition::RequestGenerateThumbnail(aAssetData, aOnThumbnailGeneratedCallback);
	}

	bool Texture2DAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Texture2D::StaticType();
	}

	bool Texture2DAssetDefinition::SupportsAsset(AssetData* aAssetData) const noexcept
	{
		return aAssetData->Type == Texture2D::StaticType();
	}

	bool Texture2DAssetDefinition::SupportsAsset(const AssetHandle& aAssetHandle) const noexcept
	{
		return aAssetHandle.Type == Texture2D::StaticType();
	}

}