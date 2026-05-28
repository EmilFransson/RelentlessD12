#pragma once

#include "Thumbnail/AssetThumbnailData.h"

#include "UI/Widgets/IWidget.h"

namespace Relentless
{
	enum class ESelectionType : uint8;

	class AssetTileItem;
	class ITableRow;
	class HorizontalBox;
	class Thumbnail;
	template<typename T>
	class TileView;

	class AssetView : public IWidget<AssetView>
	{
	public:
		AssetView() noexcept;
		virtual ~AssetView() noexcept;
	private:
		void InitializeFromAssetRegistry() noexcept;

		void OnAssetAdded(const AssetData& aAssetData) noexcept;
		NO_DISCARD Reply OnAssetTileItemDragDetected(MAYBE_UNUSED const WidgetGeometry& aGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo) noexcept;
		void OnAssetTileItemDoubleClicked(const SharedPtr<AssetThumbnailData>& aThumbnailData) noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateItem(const SharedPtr<AssetThumbnailData>& aItem) noexcept;
		NO_DISCARD const std::vector<SharedPtr<AssetThumbnailData>>* OnRequestSource() noexcept;
		void OnRender() noexcept override;
	private:
		std::vector<SharedPtr<AssetThumbnailData>> m_Items;
		Ref<TileView<SharedPtr<AssetThumbnailData>>> m_pAssetsTreeView = nullptr;

		CallbackID m_AssetRegistryFileScanDoneID = -1;
	};
}