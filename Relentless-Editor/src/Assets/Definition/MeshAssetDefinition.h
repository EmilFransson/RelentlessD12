#pragma once
#include <Relentless.h>
#include "IAssetDefinition.h"

namespace Relentless
{
	class MeshAssetDefinition : public IAssetDefinition
	{
	public:
		virtual NO_DISCARD Color GetAssetColor() const noexcept override;
		virtual NO_DISCARD String GetAssetDisplayName() const noexcept override;
		virtual NO_DISCARD TypeIndex GetSupportedAssetType() const noexcept override;
		
		virtual NO_DISCARD bool SupportsAsset(IAsset* aAsset) const noexcept override;
		virtual NO_DISCARD bool SupportsAsset(AssetData* aAssetData) const noexcept override;
	};
}