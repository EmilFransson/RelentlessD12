#pragma once
#include <Relentless.h>
#include "IAssetDefinition.h"

namespace Relentless
{
	class Texture2DAssetDefinition : public IAssetDefinition
	{
	public:
		NO_DISCARD String GetAssetDisplayName() const noexcept override;
		NO_DISCARD Color GetAssetColor() const noexcept override;
		NO_DISCARD bool SupportsAsset(IAsset* aAsset) const noexcept override;
	};
}