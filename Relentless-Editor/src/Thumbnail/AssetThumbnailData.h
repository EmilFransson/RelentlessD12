#pragma once
#include <Relentless.h>

#include "UI/Brush/Brush.h"

namespace Relentless
{
	class AssetThumbnail;
	class AssetThumbnailPool;

	class AssetThumbnailData : public SharedFromThis<AssetThumbnailData>
	{
	public:
		AssetThumbnailData(const AssetData& aAssetData, const SharedPtr<AssetThumbnailPool>& aAssetThumbnailPool) noexcept;
		virtual ~AssetThumbnailData() noexcept;

		NO_DISCARD const AssetData& GetAssetData() const noexcept;
		NO_DISCARD const ThumbnailBrush& GetBrush() const noexcept;

		NO_DISCARD Ref<AssetThumbnail> MakeThumbnailWidget(const Vector2& aSize) const noexcept;

		void SetAssetData(const AssetData& aAssetData) noexcept;

		mutable Broadcaster<void(const ThumbnailBrush&)> OnThumbnailBrushUpdated;
	private:
		void OnThumbnailRegenerated(const AssetData& aAssetData, const Ref<Texture>& aTexture) noexcept;
		
		void QueryAssetColor() noexcept;
	private:
		AssetData m_AssetData;
		ThumbnailBrush m_Brush;
		WeakPtr<AssetThumbnailPool> m_pAssetThumbnailPool;
	};
}