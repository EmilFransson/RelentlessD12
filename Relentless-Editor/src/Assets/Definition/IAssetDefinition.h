#pragma once
#include <Relentless.h>

#include "../../UI/Meta/ThumbnailInfo.h"

namespace Relentless
{
	class IAsset;

	class IAssetDefinition : public RefCounted<IAssetDefinition>
	{
	public:
		IAssetDefinition() noexcept = default;
		virtual ~IAssetDefinition() noexcept = default;

		NO_DISCARD virtual std::vector<String> GetAssetCategories() const noexcept;
		NO_DISCARD virtual String GetAssetDisplayName() const noexcept;
		NO_DISCARD virtual Color GetAssetColor() const noexcept;
		NO_DISCARD virtual TypeIndex GetSupportedAssetType() const noexcept;
		NO_DISCARD virtual Ref<ThumbnailInfo> GetThumbnailInfo(MAYBE_UNUSED const AssetData& aAssetData) const noexcept;

		NO_DISCARD virtual bool OpenAssets(MAYBE_UNUSED const std::vector<Ref<IAsset>>& someAssets) noexcept;
		
		virtual bool RequestGenerateThumbnail(MAYBE_UNUSED const AssetData& aAssetData, MAYBE_UNUSED const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept;
		
		NO_DISCARD virtual bool SupportsCreateNew() const noexcept;
		NO_DISCARD virtual bool SupportsAsset(MAYBE_UNUSED IAsset* aAsset) const noexcept;
		NO_DISCARD virtual bool SupportsAsset(MAYBE_UNUSED AssetData* aAssetData) const noexcept;
		NO_DISCARD virtual bool SupportsAsset(MAYBE_UNUSED const AssetHandle& aAssetHandle) const noexcept;
	};
}