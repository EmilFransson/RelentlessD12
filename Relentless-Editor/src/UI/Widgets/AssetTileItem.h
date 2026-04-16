#pragma once
#include "IWidget.h"

#include "AssetThumbnail.h"

#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	class AssetThumbnailData;
	template<typename T> class TileView;

	class AssetTileItem : public ITableRow
	{
	public:
		AssetTileItem(const AssetThumbnailData& aAssetThumbnailData, const Vector2& aSize, TileView<SharedPtr<AssetThumbnailData>>* pTileView) noexcept;

		const Color& GetBackgroundColor() const noexcept override;
		NO_DISCARD uint32 GetNumColumns() noexcept override;

		virtual bool SupportsDrag() const noexcept override { return true; }

		virtual AssetTileItem* SetBackgroundColor(const Color& aColor) noexcept override;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		virtual void OnRenderColumn(MAYBE_UNUSED uint32 aColumn) noexcept override;
	private:
		virtual void OnMouseButtonDown(MAYBE_UNUSED const WidgetGeometry& aGeometry, const PointerInfo& aPointerInfo) noexcept override;
		virtual void OnMouseButtonDoubleClick(MAYBE_UNUSED const WidgetGeometry& aGeometry, const PointerInfo& aPointerInfo) noexcept override;
	private:
		Color m_BackgroundColor;
		Color m_TypeColor = Colors::White;
		String m_DisplayName = "";
		String m_Name = "";
		Vector2 m_Size = Vector2::Zero;
		Ref<AssetThumbnail> m_pThumbnail;
		TileView<SharedPtr<AssetThumbnailData>>* m_pTileView = nullptr;
	};
}