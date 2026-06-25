#pragma once

#include "UI/Views/Assets/Items/AssetViewItem.h"
#include "UI/Widgets/IWidget.h"
#include "Utility/Filter/AssetFilterCollection.h"

namespace Relentless
{
	enum class ESelectionType : uint8;

	class AssetViewTile;
	class Button;
	class ContextMenu;
	class ITableRow;
	class HorizontalBox;
	class SearchBar;
	class Thumbnail;
	template<typename T> class TileView;
	class VerticalBox;

	enum class EAssetViewThumbnailSize : uint8 { Small = 0u, Medium, Large, Count };

	class AssetView : public IWidget<AssetView>
	{
	public:
		AssetView() noexcept;
		virtual ~AssetView() noexcept;

		NO_DISCARD bool BelongsToCurrentView(const String& aVirtualPath) const noexcept;

		void Clear() noexcept;
		
		NO_DISCARD AssetFilterCollection& GetAssetFilterCollection() noexcept;
		NO_DISCARD EAssetViewThumbnailSize GetThumbnailSize() const noexcept;
		NO_DISCARD uint32 GetNumItems() const noexcept;
		NO_DISCARD uint32 GetNumSelectedItems() const noexcept;

		NO_DISCARD bool IsMainViewFocused() const noexcept;
		NO_DISCARD bool IsMainViewHovered() const noexcept;
		NO_DISCARD bool IsShowingFolders() const noexcept;

		void SelectAll() noexcept;
		void SetAssetThumbnailSize(EAssetViewThumbnailSize aThumbnailSize) noexcept;
		void SetSourceFolders(const std::vector<String>& someVirtualPaths) noexcept;
		void ShowFolders(bool aShow) noexcept;
		void ShowSelectedInExplorer() noexcept;

		Broadcaster<void(const String&)> OnEnterFolderRequested;
		Broadcaster<void()> OnSelectionChanged;
		Broadcaster<void()> OnRefresh;
	private:
		NO_DISCARD Vector2 AssetThumbnailSizeEnumToSize(EAssetViewThumbnailSize aThumbnailSize) const noexcept;

		NO_DISCARD Ref<Button> BuildFilterButton() noexcept;
		NO_DISCARD Ref<SearchBar> BuildSearchBar() noexcept;
		NO_DISCARD Ref<Button> BuildSortingButton() noexcept;

		NO_DISCARD Ref<AssetViewTile> CreateAssetTile(const SharedPtr<AssetViewItem>& aItem) noexcept;
		NO_DISCARD Ref<AssetViewTile> CreateFolderTile(const SharedPtr<AssetViewItem>& aItem) noexcept;

		NO_DISCARD Ref<Thumbnail> GetFolderThumbnail(const Vector2& aSize) noexcept;
		NO_DISCARD Vector2 GetGridThumbnailSize() const noexcept;

		void InitializeFromAssetRegistry() noexcept;

		void OnAssetAdded(const AssetData& aAssetData) noexcept;
		void OnAssetTileDoubleClicked(const SharedPtr<AssetViewItem>& aItem) noexcept;
		NO_DISCARD Ref<ContextMenu> OnContextMenuOpening(MAYBE_UNUSED const SharedPtr<AssetViewItem>& aItem) noexcept;
		NO_DISCARD String OnDebugItemToString(const SharedPtr<AssetViewItem>& aItem) const noexcept;
		void OnEditSelectedAssetsClicked() noexcept;
		void OnFilterButtonClicked() noexcept;
		void OnFolderTileDoubleClick(const SharedPtr<AssetViewItem>& aItem) noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateItem(const SharedPtr<AssetViewItem>& aItem) noexcept;
		void OnNewFolderItemClicked(MAYBE_UNUSED const String& aParentVirtualPath) noexcept;
		void OnPathAdded(const String& aVirtualPath, const String& aDisplayName, MAYBE_UNUSED EAssetSourceType aSourceType) noexcept;
		NO_DISCARD const std::vector<SharedPtr<AssetViewItem>>* OnRequestSource() noexcept;
		void OnRender() noexcept override;
		void OnSearchBarTextChanged(const char* aText) noexcept;
		void OnSelectionChangedInternal(MAYBE_UNUSED const SharedPtr<AssetViewItem>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept;
		void OnShowInExplorerItemClicked() noexcept;
		void OnSortingButtonClicked() noexcept;
		NO_DISCARD Reply OnTileDragDetected(MAYBE_UNUSED AssetViewTile* aAssetViewTile) noexcept;
		void OnTileItemDoubleClicked(const SharedPtr<AssetViewItem>& aItem) noexcept;

		NO_DISCARD String ParentOf(const String& aVirtualPath) const noexcept;

		NO_DISCARD bool RequiresAssignedSize() const noexcept override;
		void Repopulate() noexcept;

		void UpdateSearchBarHintText() noexcept;
	private:
		std::vector<SharedPtr<AssetViewItem>> m_Items;
		std::vector<String> m_SourceFolders;
		
		AssetFilterCollection m_AssetFilters;
		
		Ref<TileView<SharedPtr<AssetViewItem>>> m_pAssetsTileView = nullptr;
		Ref<VerticalBox> m_pRoot = nullptr;
		HorizontalBox* m_pTileViewBox = nullptr;
		Button* m_pSortingButton = nullptr;
		SearchBar* m_pSearchBar = nullptr;
		
		bool m_SortAscending = true;
		bool m_ShowFolders = true;
		CallbackID m_AssetRegistryFileScanDoneID = -1;
		EAssetViewThumbnailSize m_ThumbnailSize = EAssetViewThumbnailSize::Medium;

		Ref<Texture2D> m_pFolderThumbnailTexture = nullptr;
	};
}