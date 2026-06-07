#pragma once
#include "UI/Widgets/IWidget.h"

#include "TableRows/PathTableRow.h"

namespace Relentless
{
	enum class ESelectionType : uint8;
	class HeaderRow;
	class SearchBar;
	template<typename T> class TreeView;
	class VerticalBox;

	class PathView : public IWidget<PathView>
	{
	public:
		PathView(AssetRegistryModule& aAssetRegistryModule) noexcept;
		virtual ~PathView() noexcept override;

		NO_DISCARD std::vector<Ref<PathListItem>> GetSelectedItems() const noexcept;
		
		NO_DISCARD bool IsMainViewFocused() const noexcept;

		void Refresh() noexcept;

		void SelectAll() noexcept;
		void SetRootFilter(Callback<bool(EAssetSourceType, const String&)>&& aRootFilter) noexcept;
		void SetSelectedItemByVirtualPath(const String& aVirtualPath) noexcept;

		Broadcaster<void()> OnSelectionChanged;
	private:
		NO_DISCARD SharedPtr<HeaderRow> BuildHeaderRow() noexcept;
		NO_DISCARD Ref<TreeView<Ref<PathListItem>>> BuildPathTreeView() noexcept;
		NO_DISCARD Ref<SearchBar> BuildSearchBar() noexcept;

		NO_DISCARD Ref<PathListItem> GetOrCreateItem(const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType) noexcept;

		NO_DISCARD String OnDebugItemToString(const Ref<PathListItem>& aItem) const noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<PathListItem>& aItem) noexcept;
		void OnGetChildren(const Ref<PathListItem>& aParentItem, std::vector<Ref<PathListItem>>& outChildren) noexcept;
		NO_DISCARD const std::vector<Ref<PathListItem>>* OnRequestSource() noexcept;
		void OnRender() noexcept override;
		void OnSearchBarTextChanged(const char* aText) noexcept;
		void OnSelectionChangedInternal(MAYBE_UNUSED const Ref<PathListItem>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept;

		NO_DISCARD bool RequiresAssignedSize() const noexcept override;
	private:
		std::unordered_map<String, Ref<PathListItem>> m_Items;
		std::vector<Ref<PathListItem>> m_RootItems;
		Callback<bool(EAssetSourceType, const String&)> m_RootFilter;
		
		UniquePtr<TextFilterExpressionEvaluator> m_pTextFilter;
		Ref<TreeView<Ref<PathListItem>>> m_pPathTreeView = nullptr;
		Ref<VerticalBox> m_pRoot = nullptr;
		VerticalBox* m_pPathTreeViewBox = nullptr;

		AssetRegistryModule& m_AssetRegistryModule;
	};
}