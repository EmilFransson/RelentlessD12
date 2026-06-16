#pragma once

#include "Thumbnail/AssetThumbnailData.h"

#include "UI/Widgets/IWidget.h"
#include "Utility/Filter/AssetFilterCollection.h"

namespace Relentless
{
	enum class ESelectionType : uint8;

	class AssetTileItem;
	class Button;
	class ContextMenu;
	class ITableRow;
	class HorizontalBox;
	class SearchBar;
	class Thumbnail;
	template<typename T> class TileView;
	class VerticalBox;

	enum class EAssetThumbnailSize : uint8 { Small = 0u, Medium, Large };

	class AssetView : public IWidget<AssetView>
	{
	public:
		AssetView() noexcept;
		virtual ~AssetView() noexcept;

		NO_DISCARD bool BelongsToCurrentView(const String& aVirtualPath) const noexcept;

		void Clear() noexcept;
		
		NO_DISCARD AssetFilterCollection& GetAssetFilterCollection() noexcept;
		NO_DISCARD EAssetThumbnailSize GetAssetThumbnailSize() const noexcept;
		NO_DISCARD uint32 GetNumItems() const noexcept;
		NO_DISCARD uint32 GetNumSelectedItems() const noexcept;

		NO_DISCARD bool IsMainViewFocused() const noexcept;
		NO_DISCARD bool IsMainViewHovered() const noexcept;

		void SelectAll() noexcept;
		void SetAssetThumbnailSize(EAssetThumbnailSize aAssetThumbnailSize) noexcept;
		void SetSourceFolders(const std::vector<String>& someVirtualPaths) noexcept;

		Broadcaster<void()> OnSelectionChanged;
		Broadcaster<void()> OnRefresh;
	private:
		NO_DISCARD Vector2 AssetThumbnailSizeEnumToSize(EAssetThumbnailSize aAssetThumbnailSize) const noexcept;

		NO_DISCARD Ref<Button> BuildFilterButton() noexcept;
		NO_DISCARD Ref<SearchBar> BuildSearchBar() noexcept;
		NO_DISCARD Ref<Button> BuildSortingButton() noexcept;

		void InitializeFromAssetRegistry() noexcept;

		void OnAssetAdded(const AssetData& aAssetData) noexcept;
		NO_DISCARD Reply OnAssetTileItemDragDetected(AssetTileItem* aAssetTileItem) noexcept;
		void OnAssetTileItemDoubleClicked(const SharedPtr<AssetThumbnailData>& aThumbnailData) noexcept;
		void OnFilterButtonClicked() noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateItem(const SharedPtr<AssetThumbnailData>& aItem) noexcept;
		void OnPathAdded(MAYBE_UNUSED const String& aVirtualPath, MAYBE_UNUSED const String& aDisplayName, MAYBE_UNUSED EAssetSourceType aSourceType) noexcept;
		NO_DISCARD const std::vector<SharedPtr<AssetThumbnailData>>* OnRequestSource() noexcept;
		void OnRender() noexcept override;
		void OnSearchBarTextChanged(const char* aText) noexcept;
		void OnSelectionChangedInternal(MAYBE_UNUSED const SharedPtr<AssetThumbnailData>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept;
		void OnSortingButtonClicked() noexcept;

		NO_DISCARD bool RequiresAssignedSize() const noexcept override;
		void Repopulate() noexcept;

		void UpdateSearchBarHintText() noexcept;
	private:
		std::vector<SharedPtr<AssetThumbnailData>> m_Items;
		std::vector<String> m_SourceFolders;
		
		AssetFilterCollection m_AssetFilters;
		
		Ref<TileView<SharedPtr<AssetThumbnailData>>> m_pAssetsTreeView = nullptr;
		Ref<VerticalBox> m_pRoot = nullptr;
		HorizontalBox* m_pTileViewBox = nullptr;
		Button* m_pSortingButton = nullptr;
		SearchBar* m_pSearchBar = nullptr;
		
		bool m_SortAscending = true;
		CallbackID m_AssetRegistryFileScanDoneID = -1;
		EAssetThumbnailSize m_AssetThumbnailSize = EAssetThumbnailSize::Medium;
	};
}