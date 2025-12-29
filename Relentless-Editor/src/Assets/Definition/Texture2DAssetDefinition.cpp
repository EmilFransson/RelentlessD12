#include "Texture2DAssetDefinition.h"

namespace Relentless
{
	String Texture2DAssetDefinition::GetAssetDisplayName() const noexcept
	{
		return "Texture2D";
	}

	Color Texture2DAssetDefinition::GetAssetColor() const noexcept
	{
		return Colors::Red;
	}

	bool Texture2DAssetDefinition::SupportsAsset(IAsset* aAsset) const noexcept
	{
		return aAsset->GetStaticType() == Texture2D::StaticType();
	}

}