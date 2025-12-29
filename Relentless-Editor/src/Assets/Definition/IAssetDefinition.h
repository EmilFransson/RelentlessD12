#pragma once

namespace Relentless
{
	class IAsset;

	class IAssetDefinition
	{
	public:
		virtual ~IAssetDefinition() noexcept = default;

		virtual NO_DISCARD std::vector<String> GetAssetCategories() const noexcept { return {"Misc"}; }
		virtual NO_DISCARD String GetAssetDisplayName() const noexcept { return "Asset"; }
		virtual NO_DISCARD Color GetAssetColor() const noexcept { return Colors::White; }
		virtual NO_DISCARD bool OpenAssets(const std::vector<Ref<IAsset>>& someAssets) noexcept { return false; };
		virtual NO_DISCARD bool SupportsCreateNew() const noexcept { return false; }
		virtual NO_DISCARD bool SupportsAsset(IAsset* aAsset) const noexcept { return false; }
	};
}