#include "MeshAssetDefinition.h"

namespace Relentless
{
	Color MeshAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::Blue;
	}

	String MeshAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Mesh";
	}

	bool MeshAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Mesh::StaticType();
	}

	bool MeshAssetDefinition::SupportsAsset(AssetData* aAssetData) const noexcept
	{
		return aAssetData->Type == Mesh::StaticType();
	}

	Ref<ThumbnailInfo> MeshAssetDefinition::GetThumbnailInfo(const AssetData& aAssetData) const noexcept
	{
		Ref<ThumbnailInfo> pThumbnailInfo = RLS_NEW ThumbnailInfo();
		pThumbnailInfo->DisplayName = GetAssetDisplayName();
		pThumbnailInfo->Label = aAssetData.Name;
		pThumbnailInfo->TypeColor = GetAssetColor();
		pThumbnailInfo->Width = 256u;
		pThumbnailInfo->Height = 256u;

		return pThumbnailInfo;
	}

	TypeIndex MeshAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return Mesh::StaticType();
	}
}