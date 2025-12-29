#include "MaterialAssetDefinition.h"

namespace Relentless
{
	std::vector<String> MaterialAssetDefinition::GetAssetCategories() const noexcept
	{
		return { "Material/Material" };
	}

	String MaterialAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Material";
	}

	Color MaterialAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::Green;
	}

	bool MaterialAssetDefinition::SupportsCreateNew() const noexcept
	{
		return true;
	}

	bool MaterialAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Material::StaticType();
	}
}
