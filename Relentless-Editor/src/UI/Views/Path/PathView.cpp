#include "PathView.h"

#include "ImGui/ImGuiFonts.h"

#include "UI/DragDrop/PathDragDropOperation.h"
#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Views/TreeView.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/SearchBar.h"

namespace Relentless
{
	/*PUBLIC*/

	PathView::PathView(AssetRegistryModule& aAssetRegistryModule) noexcept
		: m_AssetRegistryModule{ aAssetRegistryModule }
	{
		m_AssetRegistryModule.OnPathAdded.Connect(this, &PathView::OnPathAdded);
		m_AssetRegistryModule.OnPathRenamed.Connect(this, &PathView::OnPathRenamed);

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
	
	PathView::~PathView() noexcept
	{
		m_AssetRegistryModule.OnPathAdded.Detach(this);
	}

	void PathView::CreateFolder(const String& aParentVirtualPath) noexcept
	{
		String outChildFolderPath;
		if (m_AssetRegistryModule.AddPath(aParentVirtualPath, outChildFolderPath))
			m_PendingReveal = { .ParentPath = aParentVirtualPath, .ChildPath = outChildFolderPath, .WasCreated = true };

		ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
	}

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

	void PathView::ShowSelectedInExplorer() noexcept
	{
		std::vector<Ref<PathListItem>> selectedItems;
		if (m_pPathTreeView->GetSelectedItems(selectedItems) == 0u)
			return;

		AssetRegistryModule& assetRegistryModule = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		for (const Ref<PathListItem>& pItem : selectedItems)
			Platform::ShowInExplorer(assetRegistryModule.VirtualPathToAbsolutePath(pItem->VirtualPath));
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
			->OnContextMenuOpening(this, &PathView::OnContextMenuOpening)
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

	PathTableRow* PathView::GetRowFor(const Ref<PathListItem>& aItem) const noexcept
	{
		if (m_pPathTreeView->IsItemVisible(aItem))
			return static_cast<PathTableRow*>(m_pPathTreeView->GetRowWidget(aItem).Get());
	
		return nullptr;
	}

	String PathView::LeafOf(const String& aVirtualPath) const noexcept
	{
		return StringUtils::Split(aVirtualPath, '/').back();
	}

	Ref<ContextMenu> PathView::OnContextMenuOpening(const Ref<PathListItem>& aPathListItem) noexcept
	{
		ContextMenuBuilder builder;

		builder.AddItem("New Folder")
			.Icon(ICON_FA_FOLDER_PLUS)
			.Tooltip(std::format("Create a new folder in {}.", aPathListItem->VirtualPath))
			.DisabledTooltip("Can only create folders when there is a single path selected.")
			.OnClicked(Callback<void()>::Bind(this, &PathView::OnNewFolderItemClicked, aPathListItem->VirtualPath))
			.Enabled(m_pPathTreeView->GetNumItemsSelected() <= 1u);

		builder.AddSection("Folder Options")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		builder.AddItem("Show in Explorer")
			.Icon(ICON_FA_MAGNIFYING_GLASS_LOCATION)
			.Tooltip("Finds this folder on disk.")
			.OnClicked(Callback<void()>::Bind(this, &PathView::OnShowInExplorerItemClicked));

		builder.AddItem("Rename")
			.Icon(ICON_FA_PEN)
			.Tooltip("Rename the selected folder.")
			.DisabledTooltip("Select a single folder to rename.")
			.OnClicked(Callback<void()>::Bind(this, &PathView::OnRenameFolderItemClicked))
			.Enabled(!aPathListItem->IsRoot && aPathListItem->SourceType != EAssetSourceType::Engine && m_pPathTreeView->GetNumItemsSelected() == 1u);

		return builder.BuildContextMenu();
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
		
		if (!aItem->IsRoot && aItem->SourceType != EAssetSourceType::Engine)
		{
			pRow->OnRenameTextChanged(this, &PathView::OnPathRenameTextChanged);
			pRow->OnRenameTextCommitted(this, &PathView::OnPathRenameTextCommitted);
			pRow->OnDragDetected(Callback<Reply(const WidgetGeometry&, const PointerInfo&)>::Bind(this, &PathView::OnPathRowDragDetected, pRow.Get()));
		}
		
		pRow->OnDragOver(Callback<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)>::Bind(this, &PathView::OnPathRowDragOver, pRow.Get()));
		pRow->OnDragEnter(Callback<void(const WidgetGeometry&, const Ref<DragDropOperationBase>&)>::Bind(this, &PathView::OnPathRowDragEnter, pRow.Get()));
		pRow->OnDragLeave(this, &PathView::OnPathRowDragExit);
		pRow->OnDrop(Callback<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)>::Bind(this, &PathView::OnPathRowDrop, pRow.Get()));

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

	void PathView::OnNewFolderItemClicked(const String& aParentVirtualPath) noexcept
	{
		String outChildFolderPath;
		if (m_AssetRegistryModule.AddPath(aParentVirtualPath, outChildFolderPath))
			m_PendingReveal = { .ParentPath = aParentVirtualPath, .ChildPath = outChildFolderPath, .WasCreated = true };

		ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
	}

	void PathView::OnPathAdded(MAYBE_UNUSED const String& aVirtualFolderPath) noexcept
	{
		Refresh();
	}

	void PathView::OnPathRenamed(MAYBE_UNUSED const String& aOldVirtualFolder, MAYBE_UNUSED const String& aNewVirtualFolderPath) noexcept
	{
		Refresh();
	}

	void PathView::OnPathRenameTextChanged(const Ref<PathListItem>& aItem, const char* aText) noexcept
	{
		Ref<ITableRow> pTableRow = m_pPathTreeView->GetRowWidget(aItem);
		PathTableRow* pPathTableRow = static_cast<PathTableRow*>(pTableRow.Get());

		const ENameStatus status = m_AssetRegistryModule.ValidateFolderName(ParentOf(aItem->VirtualPath), aItem->DisplayName, aText);
		pPathTableRow->SetRenameFieldError(status == ENameStatus::Taken || status == ENameStatus::Invalid);
	}

	void PathView::OnPathRenameTextCommitted(const Ref<PathListItem>& aItem, const char* aText, MAYBE_UNUSED ETextCommitType aTextCommitType) noexcept
	{
		PathTableRow* pRow = GetRowFor(aItem);

		const StringView newName = aText;
		if (newName.empty() || newName == aItem->DisplayName)
		{
			if (pRow)
				pRow->ShowLabel();

			return;
		}

		const ENameStatus status = m_AssetRegistryModule.ValidateFolderName(ParentOf(aItem->VirtualPath), aItem->DisplayName, String(newName));
		if (status != ENameStatus::Ok)
		{
			if (pRow)
				pRow->ShowLabel();

			return;
		}

		const FolderOpResult result = m_AssetRegistryModule.RenameFolder(aItem->VirtualPath, String(newName));
		if (!result.IsSuccess() && pRow)
			pRow->ShowLabel();
		else
		{
			Application::Get().SubmitToMainThread([this, result]()
				{
					m_PendingReveal = { .ParentPath = ParentOf(result.ResultPath), .ChildPath = result.ResultPath };
				});
		}
	}

	Reply PathView::OnPathRowDragDetected(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo) noexcept
	{
		const Ref<PathListItem>& pItem = m_pPathTreeView->GetItemFromWidget(aRow);
		if (pItem->IsRoot)
			return Reply::Unhandled();

		if (pItem->SourceType == EAssetSourceType::Engine)
			return Reply::Unhandled();

		std::vector<Ref<PathListItem>> selectedPaths;
		if (m_pPathTreeView->GetSelectedItems(selectedPaths) == 0u)
			return Reply::Unhandled();

		std::vector<String> paths;
		paths.reserve(selectedPaths.size());

		auto secondaryDraggedPathsView = selectedPaths | std::views::transform([](const Ref<PathListItem>& aItem) -> const String& { return aItem->VirtualPath; }) 
													   | std::views::filter([path = pItem->VirtualPath](const String& aPath){ return aPath != path; });

		for (const String& path : secondaryDraggedPathsView)
			paths.push_back(path);

		paths.push_back(pItem->VirtualPath);

		const String previewText = std::format("'{}'{}", pItem->VirtualPath, paths.size() > 1 ? std::format(" and {} other{}.", paths.size() - 1, paths.size() > 2 ? "s" : "") : "");

		return Reply::Handled().BeginDragDrop(RLS_NEW PathDragDropOperation(paths, previewText));
	}

	void PathView::OnPathRowDragEnter(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<PathDragDropOperation>())
			return;
		
		const Ref<PathListItem>& pItem = m_pPathTreeView->GetItemFromWidget(aRow);
		PathDragDropOperation& pathDragDropOperation = aDragDropOperation->AsType<PathDragDropOperation>();

		const std::vector<String>& paths = pathDragDropOperation.GetPaths();
		if (pItem->SourceType == EAssetSourceType::Engine)
		{
			pathDragDropOperation.SetDrawSymbolLabel(true);
			pathDragDropOperation.SetSymbol(ICON_FA_BAN, Colors::Red);
			pathDragDropOperation.SetPreviewText("Cannot move a folder onto an engine folder.");
		}
		else if (std::ranges::any_of(paths, [&pItem](const String& aPath) { return aPath == pItem->VirtualPath; }))
		{
			pathDragDropOperation.SetDrawSymbolLabel(true);
			pathDragDropOperation.SetSymbol(ICON_FA_BAN, Colors::Red);
			pathDragDropOperation.SetPreviewText("Cannot move a folder onto itself.");
		}
		else if (std::ranges::any_of(paths, [&pItem](const String& aPath) { return pItem->VirtualPath.starts_with(aPath); }))
		{
			pathDragDropOperation.SetDrawSymbolLabel(true);
			pathDragDropOperation.SetSymbol(ICON_FA_BAN, Colors::Red);
			pathDragDropOperation.SetPreviewText("Cannot move a folder onto its descendant.");
		}
		else
		{
			const String& primaryDraggedPath = pathDragDropOperation.GetPrimaryDraggedPath();
			const uint32 numDraggedPaths = pathDragDropOperation.GetNumDraggedPaths();
			const String previewText = std::format("'Move {}'{}", primaryDraggedPath, numDraggedPaths > 1 ? std::format(" and {} other{}.", numDraggedPaths - 1, numDraggedPaths > 2 ? "s" : "") : "");

			pathDragDropOperation.SetDrawSymbolLabel(true);
			pathDragDropOperation.SetSymbol(ICON_FA_CHECK, Colors::Green);
			pathDragDropOperation.SetPreviewText(previewText);
		}
	}

	void PathView::OnPathRowDragExit(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (aDragDropOperation->IsOfType<PathDragDropOperation>())
		{
			PathDragDropOperation& pathDragDropOperation = aDragDropOperation->AsType<PathDragDropOperation>();
			const String& primaryDraggedPath = pathDragDropOperation.GetPrimaryDraggedPath();
			const uint32 numDraggedPaths = pathDragDropOperation.GetNumDraggedPaths();
			const String previewText = std::format("'{}'{}", primaryDraggedPath, numDraggedPaths > 1 ? std::format(" and {} other{}.", numDraggedPaths - 1, numDraggedPaths > 2 ? "s" : "") : "");

			pathDragDropOperation.SetDrawSymbolLabel(false);
			pathDragDropOperation.SetPreviewText(previewText);
		}
	}

	Reply PathView::OnPathRowDragOver(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<PathDragDropOperation>())
			return Reply::Unhandled();

		const Ref<PathListItem>& pItem = m_pPathTreeView->GetItemFromWidget(aRow);
		PathDragDropOperation& pathDragDropOperation = aDragDropOperation->AsType<PathDragDropOperation>();

		const std::vector<String>& paths = pathDragDropOperation.GetPaths();
		if (pItem->SourceType == EAssetSourceType::Engine)
			return Reply::Unhandled();
		if (std::ranges::any_of(paths, [&pItem](const String& aPath) { return aPath == pItem->VirtualPath || pItem->VirtualPath.starts_with(aPath); }))
			return Reply::Unhandled();

		return Reply::Handled();
	}

	Reply PathView::OnPathRowDrop(PathTableRow* aRow, MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<PathDragDropOperation>())
			return Reply::Unhandled();

		const Ref<PathListItem>& pTarget = m_pPathTreeView->GetItemFromWidget(aRow);
		const String targetParent = pTarget->VirtualPath;

		PathDragDropOperation& pathDragDropOperation = aDragDropOperation->AsType<PathDragDropOperation>();
		const std::vector<String>& dragged = pathDragDropOperation.GetPaths();

		//Move descendants first, then their ancestors, if any.
		std::vector<String> orderedPaths(dragged.begin(), dragged.end());
		std::ranges::sort(orderedPaths, [](const String& aPathA, const String& aPathB) { return aPathA.size() > aPathB.size(); });

		//TODO: Collect move reports.

		for (const String& source : orderedPaths)
		{
			const String destination = targetParent + LeafOf(source) + "/";
			m_AssetRegistryModule.MovePath(source, destination);
		}

		return Reply::Handled();
	}

	void PathView::OnPostRender() noexcept
	{
		if (!m_PendingReveal)
			return;

		if (auto parent = m_Items.find(m_PendingReveal->ParentPath); parent != m_Items.end())
		{
			if (!m_pPathTreeView->GetItemInfo(parent->second).IsExpanded)
				m_pPathTreeView->SetItemExpandedState(parent->second, true);
		}

		if (auto child = m_Items.find(m_PendingReveal->ChildPath); child != m_Items.end())
		{
			if (m_PendingReveal->WasCreated)
			{
				m_pPathTreeView->RequestScrollToItem(child->second);
				Ref<ITableRow> pTableRow = m_pPathTreeView->GetRowWidget(child->second);
				static_cast<PathTableRow*>(pTableRow.Get())->ShowEditableTextBox();
			}

			m_pPathTreeView->ClearSelection();
			m_pPathTreeView->SetItemSelection(child->second, ESelectionType::Selected);
					
		}
		
		m_PendingReveal.reset();
	}

	void PathView::OnRenameFolderItemClicked() noexcept
	{
		std::vector<Ref<PathListItem>> selectedItems;
		if (m_pPathTreeView->GetSelectedItems(selectedItems) == 1u)
		{
			Ref<ITableRow> pTableRow = m_pPathTreeView->GetRowWidget(selectedItems.front());
			PathTableRow* pPathRow = static_cast<PathTableRow*>(pTableRow.Get());
			pPathRow->ShowEditableTextBox();
		}
			
		ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
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

	void PathView::OnShowInExplorerItemClicked() noexcept
	{
		ShowSelectedInExplorer();
		ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
	}

	String PathView::ParentOf(const String& aVirtualPath) const noexcept
	{
		size_t end = aVirtualPath.size();
		if (end > 1u && aVirtualPath[end - 1u] == '/') 
			--end;
		
		const size_t lastSlash = aVirtualPath.find_last_of('/', end - 1u);

		return aVirtualPath.substr(0u, lastSlash + 1u);
	}

	bool PathView::RequiresAssignedSize() const noexcept
	{
		return true;
	}
}