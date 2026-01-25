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
		NO_DISCARD virtual Ref<ThumbnailInfo> GetThumbnailInfo(const AssetData& aAssetData) const noexcept;

		NO_DISCARD virtual bool OpenAssets(const std::vector<Ref<IAsset>>& someAssets) noexcept;
		
		virtual bool RequestGenerateThumbnail(const AssetData& aAssetData, const Callback<void(const Ref<Texture2D>&)>& aOnThumbnailGeneratedCallback) noexcept;
		
		NO_DISCARD virtual bool SupportsCreateNew() const noexcept;
		NO_DISCARD virtual bool SupportsAsset(IAsset* aAsset) const noexcept;
		NO_DISCARD virtual bool SupportsAsset(AssetData* aAssetData) const noexcept;
	};
}