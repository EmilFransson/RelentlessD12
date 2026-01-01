#include "MaterialAssetDefinition.h"

namespace Relentless
{
	std::vector<String> MaterialAssetDefinition::GetAssetCategories() const noexcept
	{
		return { "Material/Material" };
	}

	Color MaterialAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::Green;
	}

	String MaterialAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Material";
	}

	TypeIndex MaterialAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return Material::StaticType();
	}

	bool MaterialAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Material::StaticType();
	}

	bool MaterialAssetDefinition::SupportsAsset(AssetData* aAssetData) const noexcept
	{
		return aAssetData->Type == Material::StaticType();
	}

	bool MaterialAssetDefinition::SupportsCreateNew() const noexcept
	{
		return true;
	}
}
