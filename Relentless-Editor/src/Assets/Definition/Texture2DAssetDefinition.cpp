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

	bool Texture2DAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Texture2D::StaticType();
	}

	bool Texture2DAssetDefinition::SupportsAsset(AssetData* aAssetData) const noexcept
	{
		return aAssetData->Type == Texture2D::StaticType();
	}
}