#pragma once
#include <Relentless.h>

#include "IStylableWidget.h"

#include "UI/Brush/Brush.h"

namespace Relentless
{
	class AssetThumbnailData;

	class AssetThumbnail : public IStylableWidget<AssetThumbnail>
	{
	public:
		AssetThumbnail(WeakPtr<const AssetThumbnailData> aAssetThumbnailData, const Vector2& aSize) noexcept;
		virtual ~AssetThumbnail() noexcept override;

		NO_DISCARD Vector2 ReportSize() const noexcept override;
	private:
		virtual void OnRender() noexcept override;
		void OnThumbnailBrushUpdated(const ThumbnailBrush& aBrush) noexcept;
	private:
		ThumbnailBrush m_Brush;
		Vector2 m_Size = Vector2::Zero;
		WeakPtr<const AssetThumbnailData> m_pAssetThumbnailData;
	};
}