#pragma once
#include <Relentless.h>

#include "UI/Widgets/Thumbnail.h"

namespace Relentless
{
	class AssetThumbnailData;

	class AssetThumbnail : public Thumbnail
	{
	public:
		AssetThumbnail(WeakPtr<const AssetThumbnailData> aAssetThumbnailData, const Vector2& aSize) noexcept;
		virtual ~AssetThumbnail() noexcept override;
	private:
		void OnThumbnailBrushUpdated(const ThumbnailBrush& aBrush) noexcept;
	private:
		WeakPtr<const AssetThumbnailData> m_pAssetThumbnailData;
	};
}