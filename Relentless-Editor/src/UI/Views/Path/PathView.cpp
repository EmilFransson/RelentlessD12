#include "PathView.h"

#include "UI/Views/TreeView.h"
#include "UI/Widgets/Label.h"

namespace Relentless
{
	/*PUBLIC*/

	PathView::PathView(AssetRegistryModule& aAssetRegistryModule) noexcept
		: m_AssetRegistryModule{ aAssetRegistryModule }
	{
		m_pPathView = RLS_NEW TreeView<Ref<PathListItem>>(BuildHeaderRow());

		TableViewStyle style{};
		style.UseAlternatingRowColors = false;
		m_pPathView->SetStyle(style);

		m_pPathView
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
	}
	
	PathView::~PathView() noexcept = default;

	std::vector<Ref<PathListItem>> PathView::GetSelectedItems() const noexcept
	{
		std::vector<Ref<PathListItem>> selectedItems;
		selectedItems.reserve(m_pPathView->GetNumItemsSelected());

		m_pPathView->GetSelectedItems(selectedItems);
		return selectedItems;
	}

	void PathView::Refresh() noexcept
	{
		m_pPathView->ClearSelection();
		m_pPathView->RequestTreeRefresh();
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
			if (m_pPathView->GetNumItemsSelected() > 1)
				m_pPathView->ClearSelection();

			if (m_pPathView->IsItemSelected(it->second))
				return;

			m_pPathView->ClearSelection();
			m_pPathView->SetItemSelection(it->second, ESelectionType::Selected);
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
		const ItemInfo& itemInfo = m_pPathView->GetItemInfo(aItem);
		Ref<PathTableRow> pRow = RLS_NEW PathTableRow({ .DisplayName = aItem->DisplayName, .Tooltip = aItem->VirtualPath, .IsExpanded = itemInfo.IsExpanded, .HasChildren = itemInfo.HasChildren, .OwningTreeView = m_pPathView });
		pRow->SetTooltipText(aItem->VirtualPath);
		
		return pRow;
	}

	void PathView::OnGetChildren(const Ref<PathListItem>& aParentItem, std::vector<Ref<PathListItem>>& outChildren) noexcept
	{
		outChildren.clear();

		m_AssetRegistryModule.ForEachChildFolder(aParentItem->VirtualPath, [this, &outChildren](const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)
			{
				outChildren.push_back(GetOrCreateItem(aVirtualPath, aDisplayName, aSourceType));
				return true;
			});

		std::ranges::sort(outChildren, [](Ref<PathListItem>& aListItemA, Ref<PathListItem>& aListItemB)
			{
				return StringUtils::ToLower(aListItemA->DisplayName) < StringUtils::ToLower(aListItemB->DisplayName);
			});
	}

	const std::vector<Ref<PathListItem>>* PathView::OnRequestSource() noexcept
	{
		m_RootItems.clear();

		m_AssetRegistryModule.ForEachRoot([this](const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)
			{
				if (m_RootFilter.IsSet() && !m_RootFilter(aSourceType, aVirtualPath))
					return true;

				Ref<PathListItem> pItem = GetOrCreateItem(aVirtualPath, aDisplayName, aSourceType);
				pItem->IsRoot = true;

				m_RootItems.push_back(pItem);

				return true;
			});

		std::ranges::sort(m_RootItems, [](Ref<PathListItem>& aListItemA, Ref<PathListItem>& aListItemB)
			{
				return StringUtils::ToLower(aListItemA->DisplayName) < StringUtils::ToLower(aListItemB->DisplayName);
			});

		return &m_RootItems;
	}

	void PathView::OnRender() noexcept
	{
		m_pPathView->Render();
	}

	void PathView::OnSelectionChangedInternal(MAYBE_UNUSED const Ref<PathListItem>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept
	{
		OnSelectionChanged();
	}
}