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

	TypeIndex MeshAssetDefinition::GetSupportedAssetType() const noexcept
	{
		return Mesh::StaticType();
	}
}