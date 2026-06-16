#pragma once
#include "UI/Widgets/IWidget.h"

#include "TableRows/PathTableRow.h"

namespace Relentless
{
	enum class ESelectionType : uint8;
	class ContextMenu;
	class DragDropOperationBase;
	class HeaderRow;
	class SearchBar;
	template<typename T> class TreeView;
	class VerticalBox;

	class PathView : public IWidget<PathView>
	{
	public:
		PathView(AssetRegistryModule& aAssetRegistryModule) noexcept;
		virtual ~PathView() noexcept override;

		void CreateFolder(const String& aParentVirtualPath) noexcept;

		NO_DISCARD std::vector<Ref<PathListItem>> GetSelectedItems() const noexcept;
		
		NO_DISCARD bool IsMainViewFocused() const noexcept;

		NO_DISCARD String ParentOf(const String& aVirtualPath) const noexcept;
		
		void Refresh() noexcept;

		void SelectAll() noexcept;
		void SetRootFilter(Callback<bool(EAssetSourceType, const String&)>&& aRootFilter) noexcept;
		void SetSelectedItemByVirtualPath(const String& aVirtualPath) noexcept;
		void ShowSelectedInExplorer() noexcept;

		Broadcaster<void()> OnSelectionChanged;
	private:
		NO_DISCARD SharedPtr<HeaderRow> BuildHeaderRow() noexcept;
		NO_DISCARD Ref<TreeView<Ref<PathListItem>>> BuildPathTreeView() noexcept;
		NO_DISCARD Ref<SearchBar> BuildSearchBar() noexcept;

		NO_DISCARD Ref<PathListItem> GetOrCreateItem(const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType) noexcept;
		NO_DISCARD PathTableRow* GetRowFor(const Ref<PathListItem>& aItem) const noexcept;

		NO_DISCARD String LeafOf(const String& aVirtualPath) const noexcept;

		NO_DISCARD Ref<ContextMenu> OnContextMenuOpening(const Ref<PathListItem>& aPathListItem) noexcept;
		NO_DISCARD String OnDebugItemToString(const Ref<PathListItem>& aItem) const noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<PathListItem>& aItem) noexcept;
		void OnGetChildren(const Ref<PathListItem>& aParentItem, std::vector<Ref<PathListItem>>& outChildren) noexcept;
		void OnNewFolderItemClicked(const String& aParentVirtualPath) noexcept;
		void OnPathAdded(MAYBE_UNUSED const String& aVirtualFolderPath) noexcept;
		void OnPathRenamed(MAYBE_UNUSED const String& aOldVirtualFolder, MAYBE_UNUSED const String& aNewVirtualFolderPath) noexcept;
		void OnPathRenameTextChanged(const Ref<PathListItem>& aItem, const char* aText) noexcept;
		void OnPathRenameTextCommitted(const Ref<PathListItem>& aItem, const char* aText, MAYBE_UNUSED ETextCommitType aTextCommitType) noexcept;
		NO_DISCARD Reply OnPathRowDragDetected(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo) noexcept;
		void OnPathRowDragEnter(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		void OnPathRowDragExit(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnPathRowDragOver(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnPathRowDrop(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		void OnPostRender() noexcept override;
		void OnRenameFolderItemClicked() noexcept;
		NO_DISCARD const std::vector<Ref<PathListItem>>* OnRequestSource() noexcept;
		void OnRender() noexcept override;
		void OnSearchBarTextChanged(const char* aText) noexcept;
		void OnSelectionChangedInternal(MAYBE_UNUSED const Ref<PathListItem>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept;
		void OnShowInExplorerItemClicked() noexcept; 

		NO_DISCARD bool RequiresAssignedSize() const noexcept override;
	private:
		std::unordered_map<String, Ref<PathListItem>> m_Items;
		std::vector<Ref<PathListItem>> m_RootItems;
		Callback<bool(EAssetSourceType, const String&)> m_RootFilter;

		struct PendingReveal
		{
			String ParentPath;
			String ChildPath;

			bool WasCreated = false;
		};

		std::optional<PendingReveal> m_PendingReveal;
		
		UniquePtr<TextFilterExpressionEvaluator> m_pTextFilter;
		Ref<TreeView<Ref<PathListItem>>> m_pPathTreeView = nullptr;
		Ref<VerticalBox> m_pRoot = nullptr;
		VerticalBox* m_pPathTreeViewBox = nullptr;

		AssetRegistryModule& m_AssetRegistryModule;
	};
}