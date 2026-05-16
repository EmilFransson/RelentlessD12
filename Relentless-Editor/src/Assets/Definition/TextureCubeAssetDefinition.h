#pragma once
#include <Relentless.h>
#include "IAssetDefinition.h"

namespace Relentless
{
	class TextureCubeAssetDefinition : public IAssetDefinition
	{
	public:
		NO_DISCARD virtual Color GetAssetColor() const noexcept override;
		NO_DISCARD virtual String GetAssetDisplayName() const noexcept override;
		NO_DISCARD virtual TypeIndex GetSupportedAssetType() const noexcept override;

		NO_DISCARD virtual bool SupportsAsset(IAsset* aAsset) const noexcept override;
		NO_DISCARD virtual bool SupportsAsset(AssetData* aAssetData) const noexcept override;
		NO_DISCARD virtual bool SupportsAsset(const AssetHandle& aAssetHandle) const noexcept override;
	};
}