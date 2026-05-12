#pragma once
#include <Relentless.h>
#include "IAssetDefinition.h"

namespace Relentless
{
	class MaterialAssetDefinition : public IAssetDefinition
	{
	public:
		MaterialAssetDefinition() noexcept = default;
		NO_DISCARD virtual bool RequestGenerateThumbnail(const AssetData& aAssetData, const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept override;
		NO_DISCARD virtual std::vector<String> GetAssetCategories() const noexcept override;
		NO_DISCARD virtual Color GetAssetColor() const noexcept override;
		NO_DISCARD virtual String GetAssetDisplayName() const noexcept override;
		NO_DISCARD virtual TypeIndex GetSupportedAssetType() const noexcept override;
		NO_DISCARD virtual Ref<ThumbnailInfo> GetThumbnailInfo(const AssetData& aAssetData) const noexcept override;

		NO_DISCARD bool OpenAssets(const std::vector<AssetHandle>& someAssets) noexcept override;

		NO_DISCARD virtual bool SupportsAsset(IAsset* aAsset) const noexcept override;
		NO_DISCARD virtual bool SupportsAsset(AssetData* aAssetData) const noexcept override;
		NO_DISCARD virtual bool SupportsAsset(const AssetHandle& aAssetHandle) const noexcept override;
		NO_DISCARD virtual bool SupportsCreateNew() const noexcept override;
	private:
		Ref<Texture2D> m_pThumbnail = nullptr;
	};
}