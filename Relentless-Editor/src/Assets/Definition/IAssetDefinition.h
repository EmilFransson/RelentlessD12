#pragma once
#include <Relentless.h>

namespace Relentless
{
	class IAsset;

	class IAssetDefinition : public RefCounted<IAssetDefinition>
	{
	public:
		IAssetDefinition() noexcept = default;
		virtual ~IAssetDefinition() noexcept = default;

		virtual NO_DISCARD std::vector<String> GetAssetCategories() const noexcept { return {"Misc"}; }
		virtual NO_DISCARD String GetAssetDisplayName() const noexcept { return "Asset"; }
		virtual NO_DISCARD Color GetAssetColor() const noexcept { return Colors::White; }
		virtual NO_DISCARD TypeIndex GetSupportedAssetType() const noexcept { return TypeIndex{}; }
		virtual NO_DISCARD bool OpenAssets(const std::vector<Ref<IAsset>>& someAssets) noexcept { return false; };
		virtual NO_DISCARD bool SupportsCreateNew() const noexcept { return false; }
		virtual NO_DISCARD bool SupportsAsset(IAsset* aAsset) const noexcept { return false; }
		virtual NO_DISCARD bool SupportsAsset(AssetData* aAssetData) const noexcept { return false; }
	};
}