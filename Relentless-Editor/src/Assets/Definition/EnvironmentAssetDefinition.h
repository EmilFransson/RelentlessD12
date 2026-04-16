#pragma once
#include <Relentless.h>
#include "IAssetDefinition.h"

namespace Relentless
{
	class EnvironmentAssetDefinition : public IAssetDefinition
	{
	public:
		EnvironmentAssetDefinition() noexcept = default;
		NO_DISCARD virtual bool RequestGenerateThumbnail(MAYBE_UNUSED const AssetData& aAssetData, const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept override final;
		NO_DISCARD virtual std::vector<String> GetAssetCategories() const noexcept override final;
		NO_DISCARD virtual Color GetAssetColor() const noexcept override final;
		NO_DISCARD virtual String GetAssetDisplayName() const noexcept override final;
		NO_DISCARD virtual TypeIndex GetSupportedAssetType() const noexcept override final;
		NO_DISCARD virtual Ref<ThumbnailInfo> GetThumbnailInfo(const AssetData& aAssetData) const noexcept override final;

		NO_DISCARD bool OpenAssets(const std::vector<Ref<IAsset>>& someAssets) noexcept override;
		
		NO_DISCARD virtual bool SupportsAsset(IAsset* aAsset) const noexcept override final;
		NO_DISCARD virtual bool SupportsAsset(AssetData* aAssetData) const noexcept override final;
		NO_DISCARD virtual bool SupportsCreateNew() const noexcept override final;
	private:
		Ref<Texture2D> m_pThumbnail = nullptr;
	};
}