#include "AssetThumbnail.h"

#include "UI/Views/Assets/Items/AssetThumbnailData.h"

namespace Relentless
{
	AssetThumbnail::AssetThumbnail(WeakPtr<const AssetThumbnailData> aAssetThumbnailData, const Vector2& aSize) noexcept
		  : m_pAssetThumbnailData{ aAssetThumbnailData }
	{
		SharedPtr<const AssetThumbnailData> pData = aAssetThumbnailData.lock();
		RLS_ASSERT(pData, "[Thumbnail::Thumbnail]: Asset Thumbnail Data is invalid.");

		SetSize(aSize);
		SetBrush(pData->GetBrush());

		pData->OnThumbnailBrushUpdated.Connect(this, &AssetThumbnail::OnThumbnailBrushUpdated);
	}

	AssetThumbnail::~AssetThumbnail() noexcept
	{
		if (SharedPtr<const AssetThumbnailData> pData = m_pAssetThumbnailData.lock())
			pData->OnThumbnailBrushUpdated.Detach(this);
	}

	void AssetThumbnail::OnThumbnailBrushUpdated(const ThumbnailBrush& aBrush) noexcept
	{
		SetBrush(aBrush);
	}
}
