#include "TextureCubeAssetDefinition.h"

namespace Relentless
{
	Color TextureCubeAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::White;
	}

	String TextureCubeAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "TextureCube";
	}

	TypeIndex TextureCubeAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return TextureCube::StaticType();
	}

	bool TextureCubeAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == TextureCube::StaticType();
	}

	bool TextureCubeAssetDefinition::SupportsAsset(AssetData* aAssetData) const noexcept
	{
		return aAssetData->Type == TextureCube::StaticType();
	}

	bool TextureCubeAssetDefinition::SupportsAsset(const AssetHandle& aAssetHandle) const noexcept
	{
		return aAssetHandle.Type == TextureCube::StaticType();
	}
}