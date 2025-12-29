#pragma once
#include <Relentless.h>
#include "IAssetDefinition.h"

namespace Relentless
{
	class MaterialAssetDefinition : public IAssetDefinition
	{
	public:
		virtual NO_DISCARD std::vector<String> GetAssetCategories() const noexcept override;
		virtual NO_DISCARD String GetAssetDisplayName() const noexcept override;
		virtual NO_DISCARD Color GetAssetColor() const noexcept override;
		virtual NO_DISCARD bool SupportsCreateNew() const noexcept override;
		virtual NO_DISCARD bool SupportsAsset(IAsset* aAsset) const noexcept override;
	};
}