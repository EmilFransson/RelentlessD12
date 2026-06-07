#include "PathView.h"

#include "ImGui/ImGuiFonts.h"

#include "UI/Views/TreeView.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/SearchBar.h"

namespace Relentless
{
	/*PUBLIC*/

	PathView::PathView(AssetRegistryModule& aAssetRegistryModule) noexcept
		: m_AssetRegistryModule{ aAssetRegistryModule }
	{
		m_pTextFilter = MakeUnique<TextFilterExpressionEvaluator>();

		m_pRoot = RLS_NEW VerticalBox();
		m_pRoot->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pRoot->SetBackgroundColor(Colors::EvenRowColorDefault);

		m_pRoot->AddWidget(BuildSearchBar());

		m_pPathTreeViewBox = m_pRoot->AddWidget(RLS_NEW VerticalBox());
		m_pPathTreeViewBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pPathTreeViewBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pPathTreeViewBox->SetMargin(FloatRect::WithTop(10.0f));

		m_pPathTreeViewBox->AddWidget(BuildPathTreeView());
	}
	
	PathView::~PathView() noexcept = default;

	std::vector<Ref<PathListItem>> PathView::GetSelectedItems() const noexcept
	{
		std::vector<Ref<PathListItem>> selectedItems;
		selectedItems.reserve(m_pPathTreeView->GetNumItemsSelected());

		m_pPathTreeView->GetSelectedItems(selectedItems);
		return selectedItems;
	}

	bool PathView::IsMainViewFocused() const noexcept
	{
		return m_pPathTreeViewBox->IsFocused();
	}

	void PathView::Refresh() noexcept
	{
		m_pPathTreeView->ClearSelection();
		m_pPathTreeView->RequestTreeRefresh();
	}

	void PathView::SelectAll() noexcept
	{
		m_pPathTreeView->SelectAll();
	}

	void PathView::SetRootFilter(Callback<bool(EAssetSourceType, const String&)>&& aRootFilter) noexcept
	{
		m_RootFilter = std::move(aRootFilter);
		Refresh();
	}

	void PathView::SetSelectedItemByVirtualPath(const String& aVirtualPath) noexcept
	{
		if (auto it = m_Items.find(aVirtualPath); it != m_Items.end())
		{
			if (m_pPathTreeView->GetNumItemsSelected() > 1)
				m_pPathTreeView->ClearSelection();

			if (m_pPathTreeView->IsItemSelected(it->second))
				return;

			m_pPathTreeView->ClearSelection();
			m_pPathTreeView->SetItemSelection(it->second, ESelectionType::Selected);
		}
	}

	/*PRIVATE*/

	SharedPtr<HeaderRow> PathView::BuildHeaderRow() noexcept
	{
		SharedPtr<HeaderRow> pHeaderRow = MakeShared<HeaderRow>();
		pHeaderRow->SetIsVisible(false);

		Column column;
		column.pBox->AddWidget(RLS_NEW Label("Item Label"))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		pHeaderRow->AddColumn(column);

		return pHeaderRow;
	}

	Ref<TreeView<Ref<PathListItem>>> PathView::BuildPathTreeView() noexcept
	{
		Ref<TreeView<Ref<PathListItem>>> pPathTreeView = RLS_NEW TreeView<Ref<PathListItem>>(BuildHeaderRow());

		TableViewStyle style{};
		style.UseAlternatingRowColors = false;

		pPathTreeView->SetStyle(style);

		pPathTreeView
			->OnGetChildren(this, &PathView::OnGetChildren)
			->SetClippingActive(true)
			->OnRequestSource(this, &PathView::OnRequestSource)
			->OnGenerateRow(this, &PathView::OnGenerateRow)
			->OnDebugItemToString(this, &PathView::OnDebugItemToString)
			->OnSelectionChanged(this, &PathView::OnSelectionChangedInternal)
			->ClearSelectionOnClick(false)
			->SetSpacing(Vector2::Zero)
			->SetFlags(ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_ScrollY)
			->SetHorizontalSizePolicy(ESizePolicy::Stretch)
			->SetVerticalSizePolicy(ESizePolicy::Stretch);

		m_pPathTreeView = pPathTreeView;

		return pPathTreeView;
	}

	Ref<SearchBar> PathView::BuildSearchBar() noexcept
	{
		Ref<SearchBar> pSearchBar = RLS_NEW SearchBar("Search path");
		pSearchBar->SetFont(UI::Fonts::Get("Medium"));
		pSearchBar->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pSearchBar->OnTextChanged(this, &PathView::OnSearchBarTextChanged);
	
		return pSearchBar;
	}

	Ref<PathListItem> PathView::GetOrCreateItem(const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType) noexcept
	{
		if (auto it = m_Items.find(aVirtualPath); it != m_Items.end())
			return it->second;

		Ref<PathListItem> pItem = RLS_NEW PathListItem();
		pItem->VirtualPath = aVirtualPath;
		pItem->DisplayName = aDisplayName;
		pItem->SourceType = aSourceType;

		m_Items.emplace(aVirtualPath, pItem);
		return pItem;
	}

	String PathView::OnDebugItemToString(const Ref<PathListItem>& aItem) const noexcept
	{
		return aItem->VirtualPath;
	}

	Ref<ITableRow> PathView::OnGenerateRow(const Ref<PathListItem>& aItem) noexcept
	{
		const ItemInfo& itemInfo = m_pPathTreeView->GetItemInfo(aItem);
		const String highlightText = !m_pTextFilter->GetFilterText().empty() && m_pTextFilter->TestTextFilter(aItem->DisplayName, ETextFilterTextComparisonMode::Partial) ? m_pTextFilter->GetFilterText() : "";

		Ref<PathTableRow> pRow = RLS_NEW PathTableRow({ .DisplayName = aItem->DisplayName, .Tooltip = aItem->VirtualPath, .HighlightText = highlightText, .IsExpanded = itemInfo.IsExpanded, .HasChildren = itemInfo.HasChildren, .OwningTreeView = m_pPathTreeView });
		pRow->SetTooltipText(aItem->VirtualPath);
		
		return pRow;
	}

	void PathView::OnGetChildren(const Ref<PathListItem>& aParentItem, std::vector<Ref<PathListItem>>& outChildren) noexcept
	{
		outChildren.clear();

		struct ChildItem
		{
			String VirtualPath;
			String DisplayName;
			EAssetSourceType SourceType;
		};

		std::vector<ChildItem> children;

		m_AssetRegistryModule.ForEachChildFolder(aParentItem->VirtualPath, [&children](const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)
			{
				children.push_back({ .VirtualPath = aVirtualPath, .DisplayName = aDisplayName, .SourceType = aSourceType } );
				return true;
			});

		for (const ChildItem& child : children)
		{
			bool matchTextFilter = false;
			m_AssetRegistryModule.ForEachDescendantFolder(child.VirtualPath, [this, &matchTextFilter](const String& aVirtualPath, MAYBE_UNUSED const String& aDisplayName, MAYBE_UNUSED EAssetSourceType aSourceType)
				{
					if (m_pTextFilter->TestTextFilter(aVirtualPath, ETextFilterTextComparisonMode::Partial))
					{
						matchTextFilter = true;
						return false;
					}

					return true;
				});

			if (!matchTextFilter && !m_pTextFilter->TestTextFilter(child.VirtualPath, ETextFilterTextComparisonMode::Partial))
				continue;

			outChildren.push_back(GetOrCreateItem(child.VirtualPath, child.DisplayName, child.SourceType));
		}

		std::ranges::sort(outChildren, [](Ref<PathListItem>& aListItemA, Ref<PathListItem>& aListItemB)
			{
				return StringUtils::ToLower(aListItemA->DisplayName) < StringUtils::ToLower(aListItemB->DisplayName);
			});
	}

	const std::vector<Ref<PathListItem>>* PathView::OnRequestSource() noexcept
	{
		m_RootItems.clear();

		struct RootItem
		{
			String VirtualPath;
			String DisplayName;
			EAssetSourceType SourceType;
		};

		std::vector<RootItem> roots;

		m_AssetRegistryModule.ForEachRoot([&roots](const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)
			{
				roots.push_back({ .VirtualPath = aVirtualPath, .DisplayName = aDisplayName, .SourceType = aSourceType });
				return true;
			});

		for (const RootItem& root : roots)
		{
			if (m_RootFilter.IsSet() && !m_RootFilter(root.SourceType, root.VirtualPath))
				continue;

			bool matchTextFilter = false;
			m_AssetRegistryModule.ForEachDescendantFolder(root.VirtualPath, [this, &matchTextFilter](const String& aVirtualPath, MAYBE_UNUSED const String& aDisplayName, MAYBE_UNUSED EAssetSourceType aSourceType)
				{
					if (m_pTextFilter->TestTextFilter(aVirtualPath, ETextFilterTextComparisonMode::Partial))
					{
						matchTextFilter = true;
						return false;
					}

					return true;
				});

			if (!matchTextFilter && !m_pTextFilter->TestTextFilter(root.VirtualPath, ETextFilterTextComparisonMode::Partial))
				continue;

			Ref<PathListItem> pItem = GetOrCreateItem(root.VirtualPath, root.DisplayName, root.SourceType);
			pItem->IsRoot = true;
				
			m_RootItems.push_back(pItem);
		}

		std::ranges::sort(m_RootItems, [](Ref<PathListItem>& aListItemA, Ref<PathListItem>& aListItemB)
			{
				return StringUtils::ToLower(aListItemA->DisplayName) < StringUtils::ToLower(aListItemB->DisplayName);
			});

		return &m_RootItems;
	}

	void PathView::OnRender() noexcept
	{
		m_pRoot->AssignSize(GetAssignedSize());
		m_pRoot->Render();
	}

	void PathView::OnSearchBarTextChanged(const char* aText) noexcept
	{
		m_pTextFilter->SetFilterText(aText);
		Refresh();
	}

	void PathView::OnSelectionChangedInternal(MAYBE_UNUSED const Ref<PathListItem>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept
	{
		OnSelectionChanged();
	}

	bool PathView::RequiresAssignedSize() const noexcept
	{
		return true;
	}
}