#pragma once
#include "IWidget.h"

#include "UI/Widgets/ITableRow.h"
#include "UI/Widgets/Thumbnail.h"

namespace Relentless
{
	class AssetViewItem;
	template<typename T> class TileView;
	class VerticalBox;

	struct AssetViewTileCreateInfo
	{
		String Name					= "Name";
		String DisplayName			= "DisplayName";
		String HighlightedSubstring	= "";
		Vector2 Size				= Vector2::Zero;
		Ref<Thumbnail> Thumbnail	= nullptr;
		bool IsAssetTile			= true;
	};

	class AssetViewTile : public ITableRow
	{
	public:
		AssetViewTile(const AssetViewTileCreateInfo& aCreateInfo, TileView<SharedPtr<AssetViewItem>>* aTileView) noexcept;
		virtual ~AssetViewTile() noexcept override;

		NO_DISCARD uint32 GetNumColumns() noexcept override;

		void OnRenderColumn(MAYBE_UNUSED uint32 aColumn) noexcept override;
		
		NO_DISCARD Vector2 ReportSize() const noexcept override;
		
		AssetViewTile* SetDefaultBackgroundColor(const Color& aColor) noexcept;
		bool SupportsDrag() const noexcept override { return true; }
	protected:
		void HandleDragDrop() noexcept override;
	private:
		NO_DISCARD bool IsSelected() const noexcept;

		void OnMouseEnterTile(VerticalBox* aTileBox) noexcept;
		void OnMouseExitTile(VerticalBox* aTileBox) noexcept;
		void OnTileDeselected() noexcept;
		void OnTileMouseDown(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept;
		void OnTileMouseDoubleClick(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept;
		void OnTileMouseUp(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const PointerInfo& aPointerInfo) noexcept;
		void OnTileSelected() noexcept;
	private:
		Color m_DefaultBackgroundColor = Colors::Normalize(56.0f, 56.0f, 56.0f, 255.0f);

		TileView<SharedPtr<AssetViewItem>>* m_pTileView = nullptr;
		Ref<Thumbnail> m_pThumbnail;

		Ref<VerticalBox> m_pRoot;
		Ref<VerticalBox> m_pTileBox;

		bool m_IsSelected = false;
		bool m_IsAssetTile = true;
	};
}