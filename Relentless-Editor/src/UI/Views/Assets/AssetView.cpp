#include "AssetView.h"
#include "Assets/Factory/TextureFactory.h"

#include "Core/Editor.h"

#include "ImGui/ImGuiFonts.h"

#include "Module/ContentBrowserModule.h"

#include "Subsystem/AssetDefinitionRegistry.h"

#include "Thumbnail/AssetThumbnailPool.h"

#include "UI/DragDrop/AssetViewDragDropOperation.h"
#include "UI/Views/Assets/Items/AssetThumbnailData.h"
#include "UI/Views/Assets/Items/FolderThumbnailData.h"
#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Views/TileView.h"
#include "UI/Widgets/AssetThumbnail.h"
#include "UI/Widgets/AssetTileItemTooltip.h"
#include "UI/Widgets/AssetViewTile.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/ITableRow.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/SearchBar.h"
#include "UI/Widgets/VerticalBox.h"

#include "Utility/Filter/AssetSourceFilter.h"
#include "Utility/Filter/AssetTextFilter.h"
#include "Utility/Filter/AssetTypeFilter.h"

namespace Relentless
{
	AssetView::AssetView() noexcept
	{
		m_AssetFilters.Add<AssetTextFilter>();
		m_AssetFilters.Add<AssetTypeFilter>();
		m_AssetFilters.Add<AssetSourceFilter>();
		m_AssetFilters.OnFilterChanged.Connect(this, &AssetView::Repopulate);

		m_pRoot = RLS_NEW VerticalBox();
		m_pRoot->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);

		HorizontalBox* pChromeBox = m_pRoot->AddWidget(RLS_NEW HorizontalBox());
		pChromeBox->SetMargin(FloatRect::WithBottom(10.0f));
		pChromeBox->AddWidget(BuildFilterButton());
		pChromeBox->AddWidget(BuildSearchBar());
		pChromeBox->AddWidget(BuildSortingButton());
		
		m_pTileViewBox = m_pRoot->AddWidget(RLS_NEW HorizontalBox());
		m_pTileViewBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pTileViewBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pTileViewBox->SetMouseScrollingEnabled(true);
		m_pTileViewBox->SetScrollBarsVisible(true);

		const Vector2 size = AssetThumbnailSizeEnumToSize(m_ThumbnailSize);
		
		m_pAssetsTileView = m_pTileViewBox->AddWidget(RLS_NEW TileView<SharedPtr<AssetViewItem>>());

		m_pAssetsTileView
			->SetItemWidth(size.x)
			->SetItemHeight(size.y)
			->OnDebugItemToString(this, &AssetView::OnDebugItemToString)
			->OnRequestSource(this, &AssetView::OnRequestSource)
			->OnGenerateRow(this, &AssetView::OnGenerateItem)
			->OnSelectionChanged(this, &AssetView::OnSelectionChangedInternal)
			->OnDoubleClick(this, &AssetView::OnTileItemDoubleClicked)
			->OnContextMenuOpening(this, &AssetView::OnContextMenuOpening)
			->SetHorizontalSizePolicy(ESizePolicy::Stretch)
			->SetVerticalSizePolicy(ESizePolicy::Stretch);

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		if (!assetRegistry.IsLoadingAssets())
			InitializeFromAssetRegistry();
		else
		{
			m_AssetRegistryFileScanDoneID = assetRegistry.OnFileScanDone.Connect([this]()
				{
					InitializeFromAssetRegistry();
					ModuleManager::LoadModuleChecked<AssetRegistryModule>().OnFileScanDone.Detach(m_AssetRegistryFileScanDoneID);
					m_AssetRegistryFileScanDoneID = -1;
				});
		}
	}

	AssetView::~AssetView() noexcept
	{
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistry.OnAssetAdded.Detach(this);
	}

	bool AssetView::BelongsToCurrentView(const String& aVirtualPath) const noexcept
	{
		if (m_SourceFolders.empty())
			return true;

		for (const String& src : m_SourceFolders)
		{
			if (aVirtualPath == src)
				return true;
		}

		return false;
	}

	void AssetView::Clear() noexcept
	{
		m_Items.clear();
		m_pAssetsTileView->ClearItemsSource();
		m_pAssetsTileView->ClearSelection();
		m_pAssetsTileView->RequestRefresh();
	}

	AssetFilterCollection& AssetView::GetAssetFilterCollection() noexcept
	{
		return m_AssetFilters;
	}

	EAssetViewThumbnailSize AssetView::GetThumbnailSize() const noexcept
	{
		return m_ThumbnailSize;
	}

	uint32 AssetView::GetNumItems() const noexcept
	{
		return static_cast<uint32>(m_Items.size());
	}

	uint32 AssetView::GetNumSelectedItems() const noexcept
	{
		return m_pAssetsTileView->GetNumItemsSelected();
	}

	bool AssetView::IsMainViewFocused() const noexcept
	{
		return m_pTileViewBox->IsFocused();
	}

	bool AssetView::IsMainViewHovered() const noexcept
	{
		return m_pTileViewBox->IsHovered();
	}

	bool AssetView::IsShowingFolders() const noexcept
	{
		return m_ShowFolders;
	}

	void AssetView::SelectAll() noexcept
	{
		m_pAssetsTileView->SelectAll();
	}

	void AssetView::SetAssetThumbnailSize(EAssetViewThumbnailSize aThumbnailSize) noexcept
	{
		if (m_ThumbnailSize == aThumbnailSize)
			return;

		m_ThumbnailSize = aThumbnailSize;

		const Vector2 size = AssetThumbnailSizeEnumToSize(m_ThumbnailSize);
		m_pAssetsTileView->SetItemWidth(size.x);
		m_pAssetsTileView->SetItemHeight(size.y);
		Repopulate();
	}

	void AssetView::SetSourceFolders(const std::vector<String>& someVirtualPaths) noexcept
	{
		m_SourceFolders = someVirtualPaths;

		Repopulate();
		UpdateSearchBarHintText();
	}

	void AssetView::ShowFolders(bool aShow) noexcept
	{
		if (m_ShowFolders == aShow)
			return;

		m_ShowFolders = aShow;
		Repopulate();
	}

	void AssetView::ShowSelectedInExplorer() noexcept
	{
		std::vector<SharedPtr<AssetViewItem>> selectedItems;
		if (m_pAssetsTileView->GetSelectedItems(selectedItems) == 0u)
			return;

		AssetRegistryModule& assetRegistryModule = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		for (const SharedPtr<AssetViewItem>& pItem : selectedItems)
		{
			if (pItem->GetType() == EAssetViewItemType::Folder)
			{
				FolderThumbnailData* pFolderThumbnailData = static_cast<FolderThumbnailData*>(pItem.get());
				Platform::ShowInExplorer(assetRegistryModule.VirtualPathToAbsolutePath(pFolderThumbnailData->GetVirtualPath()));
			}
			else
			{
				AssetThumbnailData* pAssetThumbnailData = static_cast<AssetThumbnailData*>(pItem.get());
				const AssetData& assetData = pAssetThumbnailData->GetAssetData();
				const String absolutePath = assetRegistryModule.VirtualPathToAbsolutePath(assetData.PackagePath.string());
				const String fileLocation = absolutePath + "\\" + assetData.Name + ".rasset";
				Platform::ShowInExplorer(fileLocation);
			}
		}
	}

	Vector2 AssetView::AssetThumbnailSizeEnumToSize(EAssetViewThumbnailSize aThumbnailSize) const noexcept
	{
		switch (aThumbnailSize)
		{
		case EAssetViewThumbnailSize::Small: return Vector2(75.0f, 127.5f);
		case EAssetViewThumbnailSize::Medium: return Vector2(100.0f, 170.0f);
		case EAssetViewThumbnailSize::Large: return Vector2(125.0f, 212.5f);
		default: RLS_ASSERT(false, "[AssetView::AssetThumbnailSizeEnumToSize]: Unknown asset thumbnail size encountered."); return Vector2(100.0f, 170.0f);
		}
	}

	Ref<Button> AssetView::BuildFilterButton() noexcept
	{
		Ref<Button> pFilterButton = Button::CreateTransparent(std::format("{} {}", ICON_FA_FILTER, ICON_FA_CHEVRON_DOWN));
		pFilterButton->SetPadding(Vector2(8.0f, 4.0f));
		pFilterButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pFilterButton->SetTooltipText("Open the Add Filter Menu to manage filters.");
		pFilterButton->SetTextColor(Colors::TextInactive);
		pFilterButton->OnClicked(this, &AssetView::OnFilterButtonClicked);
		pFilterButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
		pFilterButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });

		return pFilterButton;
	}

	Ref<SearchBar> AssetView::BuildSearchBar() noexcept
	{
		Ref<SearchBar> pSearchBar = RLS_NEW SearchBar("Search Assets", true);
		pSearchBar->SetFont(UI::Fonts::Get("Medium"));
		pSearchBar->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pSearchBar->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pSearchBar->SetMaxWidth(600.0f);
		pSearchBar->OnTextChanged(this, &AssetView::OnSearchBarTextChanged);

		m_pSearchBar = pSearchBar;

		return pSearchBar;
	}

	Ref<Button> AssetView::BuildSortingButton() noexcept
	{
		Ref<Button> pSortingButton = Button::CreateTransparent(std::format("{} {}", ICON_FA_ARROW_DOWN_SHORT_WIDE, ICON_FA_CHEVRON_DOWN));
		pSortingButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pSortingButton->SetPadding(Vector2(8.0f, 4.0f));
		pSortingButton->SetTooltipText("Sorting options for the current asset view.");
		pSortingButton->SetTextColor(Colors::TextInactive);
		pSortingButton->OnClicked(this, &AssetView::OnSortingButtonClicked);
		pSortingButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
		pSortingButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });

		m_pSortingButton = pSortingButton;

		return pSortingButton;
	}

	Ref<AssetViewTile> AssetView::CreateAssetTile(const SharedPtr<AssetViewItem>& aItem) noexcept
	{
		const AssetThumbnailData* pAssetThumbnailData = static_cast<const AssetThumbnailData*>(aItem.get());
		const AssetData& assetData = pAssetThumbnailData->GetAssetData();

		const AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		const IAssetDefinition* pAssetDefinition = pAssetDefinitionRegistry->GetDefinitionForAsset(assetData);
		RLS_ASSERT(pAssetDefinition, "[AssetView::CreateAssetTile]: Asset Definition is invalid for asset.");

		const Vector2 tileSize = AssetThumbnailSizeEnumToSize(m_ThumbnailSize);
		const Vector2 thumbnailSize = Vector2(tileSize.x - 10.0f, (tileSize.y * 0.5f) - 4.0f);

		AssetViewTileCreateInfo createInfo
		{
			.Name = assetData.Name,
			.DisplayName = pAssetDefinition->GetAssetDisplayName(),
			.HighlightedSubstring = m_AssetFilters.Get<AssetTextFilter>()->GetFilterText(),
			.Size = tileSize,
			.Thumbnail = pAssetThumbnailData->MakeThumbnailWidget(thumbnailSize),
			.IsAssetTile = true
		};

		Ref<AssetViewTile> pAssetTile = RLS_NEW AssetViewTile(createInfo, m_pAssetsTileView);
		pAssetTile->SetTooltip(RLS_NEW AssetTileItemTooltip(assetData));
		pAssetTile->OnDragDetected([this, pItem = pAssetTile.Get()](MAYBE_UNUSED const WidgetGeometry& aGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo)
			{
				return OnTileDragDetected(pItem);
			});

		return pAssetTile;
	}

	Ref<AssetViewTile> AssetView::CreateFolderTile(const SharedPtr<AssetViewItem>& aItem) noexcept
	{
		const Vector2 tileSize = AssetThumbnailSizeEnumToSize(m_ThumbnailSize);

		FolderThumbnailData* pFolderThumbnailData = static_cast<FolderThumbnailData*>(aItem.get());

		AssetViewTileCreateInfo createInfo
		{
			.Name = pFolderThumbnailData->GetName(),
			.DisplayName = "",
			.HighlightedSubstring = m_AssetFilters.Get<AssetTextFilter>()->GetFilterText(),
			.Size = tileSize,
			.Thumbnail = GetFolderThumbnail(GetGridThumbnailSize()),
			.IsAssetTile = false
		};

		Ref<AssetViewTile> pFolderTile = RLS_NEW AssetViewTile(createInfo, m_pAssetsTileView);
		pFolderTile->SetTooltipText(pFolderThumbnailData->GetVirtualPath());
		pFolderTile->OnDragDetected([this, pItem = pFolderTile.Get()](MAYBE_UNUSED const WidgetGeometry& aGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo)
			{
				return OnTileDragDetected(pItem);
			});

		return pFolderTile;
	}

	Ref<Thumbnail> AssetView::GetFolderThumbnail(const Vector2& aSize) noexcept
	{
		if (!m_pFolderThumbnailTexture)
		{
			std::vector<AssetImportTask> importTasks;

			AssetImportTask& task = importTasks.emplace_back();
			task.FilePath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/folder_256x256.png");
			task.ShouldSave = false;

			Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
			pFactory->SetImportAsSRGB(true);
			pFactory->SetGenerateMipmaps(true);
			task.pFactory = pFactory;

			AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
			std::vector<AssetImportResult> result = assetTools.Import(importTasks);
			m_pFolderThumbnailTexture = AssetManager::Get<Texture2D>(result[0].Handle);
			m_pFolderThumbnailTexture->CreateResource();
		}

		ThumbnailBrush brush;
		brush.BackingTexture = m_pFolderThumbnailTexture->GetResource();
		brush.TintColor = Colors::FolderDefault;

		Ref<Thumbnail> pFolderThumbnail = RLS_NEW Thumbnail();
		pFolderThumbnail->SetBrush(brush);
		pFolderThumbnail->SetSize(aSize);

		return pFolderThumbnail;
	}

	Vector2 AssetView::GetGridThumbnailSize() const noexcept
	{
		const Vector2 tileSize = AssetThumbnailSizeEnumToSize(m_ThumbnailSize);
		constexpr float WIDTH_OFFSET = 10.0f;
		constexpr float HEIGHT_OFFSET = 4.0f;

		return Vector2(tileSize.x - WIDTH_OFFSET, (tileSize.y * 0.5f) - HEIGHT_OFFSET);
	}

	void AssetView::InitializeFromAssetRegistry() noexcept
	{
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		std::vector<String> roots;

		assetRegistry.ForEachRoot([&roots](const String& aVirtualPath, const String&, EAssetSourceType)
			{
				roots.push_back(aVirtualPath);
				return true;
			});

		for (const String& root : roots)
		{
			assetRegistry.ForEachAssetWithPath(root, [this](const AssetData& aData)
				{
					OnAssetAdded(aData);
					return true;
				},
				true);
		}

		assetRegistry.OnAssetAdded.Connect(this, &AssetView::OnAssetAdded);
	}

	void AssetView::OnAssetAdded(const AssetData& aAssetData) noexcept
	{
		if (!m_AssetFilters.PassesAll(aAssetData))
			return;
		if (!BelongsToCurrentView(aAssetData.PackagePath.string()))
			return;

		ContentBrowserModule& contentBrowser = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		const SharedPtr<AssetThumbnailPool>& pThumbnailPool = contentBrowser.GetAssetThumbnailPool();

		m_Items.push_back(MakeShared<AssetThumbnailData>(aAssetData, pThumbnailPool));
		m_pAssetsTileView->RequestRefresh();
	}

	void AssetView::OnAssetTileDoubleClicked(const SharedPtr<AssetViewItem>& aItem) noexcept
	{
		AssetThumbnailData* pAssetThumbnailData = static_cast<AssetThumbnailData*>(aItem.get());

		if (const Ref<IAssetDefinition>& pAssetDefinition = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>()->GetDefinitionForAsset(pAssetThumbnailData->GetAssetData()))
		{
			const AssetHandle assetHandle = AssetManager::LoadAsset(pAssetThumbnailData->GetAssetData());
			if (!assetHandle.IsValid())
				return;

			pAssetDefinition->OpenAssets({ assetHandle });
		}
	}

	Ref<ContextMenu> AssetView::OnContextMenuOpening(MAYBE_UNUSED const SharedPtr<AssetViewItem>& aItem) noexcept
	{
		std::vector<SharedPtr<AssetViewItem>> selectedItems;
		m_pAssetsTileView->GetSelectedItems(selectedItems);
		
		RLS_ASSERT(!selectedItems.empty(), "[AssetView::OnContextMenuOpening]: Selection empty on opening context menu.");

		ContextMenuBuilder builder;
		
		const bool anyIsFolder = std::ranges::any_of(selectedItems, [](const SharedPtr<AssetViewItem>& aItem) { return aItem->GetType() == EAssetViewItemType::Folder; });

		if (anyIsFolder)
		{
			auto folderItems = selectedItems
				| std::views::filter([](const SharedPtr<AssetViewItem>& aItem) { return aItem->GetType() == EAssetViewItemType::Folder; })
				| std::views::transform([](const SharedPtr<AssetViewItem>& aItem) -> FolderThumbnailData* { return static_cast<FolderThumbnailData*>(aItem.get()); })
				| std::ranges::to<std::vector<FolderThumbnailData*>>();

			builder.AddItem("New Folder")
				.Icon(ICON_FA_FOLDER_PLUS)
				.Tooltip(std::format("Create a new folder in {}.", folderItems.front()->GetVirtualPath()))
				.DisabledTooltip("Can only create folders when there is a single path selected.")
				.OnClicked(Callback<void()>::Bind(this, &AssetView::OnNewFolderItemClicked, folderItems.front()->GetVirtualPath()))
				.Enabled(folderItems.size() == 1u);
			
			builder.AddSection("Folder Options")
				.Font(UI::Fonts::Get("Small"))
				.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
				.TextColor(Colors::TextInactive)
				.Thickness(0.5f);

			builder.AddItem("Show in Explorer")
				.Icon(ICON_FA_MAGNIFYING_GLASS_LOCATION)
				.Tooltip("Finds this folder on disk.")
				.OnClicked(Callback<void()>::Bind(this, &AssetView::OnShowInExplorerItemClicked));
		}
		else
		{
			builder.AddSection("Common")
				.Font(UI::Fonts::Get("Small"))
				.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
				.TextColor(Colors::TextInactive)
				.Thickness(0.5f);

			builder.AddItem("Edit...")
				.Icon(ICON_FA_PEN)
				.Tooltip("Opens the selected asset(s) for edit.")
				.Shortcut("Enter")
				.OnClicked(Callback<void()>::Bind(this, &AssetView::OnEditSelectedAssetsClicked));

			builder.AddSection("Explore")
				.Font(UI::Fonts::Get("Small"))
				.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
				.TextColor(Colors::TextInactive)
				.Thickness(0.5f);

			builder.AddItem("Show in Explorer")
				.Icon(ICON_FA_MAGNIFYING_GLASS_LOCATION)
				.Tooltip("Finds this folder on disk.")
				.OnClicked(Callback<void()>::Bind(this, &AssetView::OnShowInExplorerItemClicked));
		}

		return builder.BuildContextMenu();
	}

	String AssetView::OnDebugItemToString(const SharedPtr<AssetViewItem>& aItem) const noexcept
	{
		const String typeString = aItem->GetType() == EAssetViewItemType::Asset ? "Asset" : "Folder";
		return std::format("{} ({})", aItem->GetName(), typeString);
	}

	//Note: can only trigger if all selected items are assets.
	void AssetView::OnEditSelectedAssetsClicked() noexcept
	{
		std::vector<SharedPtr<AssetViewItem>> selectedItems;
		if (m_pAssetsTileView->GetSelectedItems(selectedItems) == 0u)
			return;

		auto assetDataView = selectedItems | std::views::transform([](const SharedPtr<AssetViewItem> aItem) -> const AssetData& 
			{  
				RLS_ASSERT(aItem->GetType() == EAssetViewItemType::Asset, "[AssetView::OnEditSelectedAssetsClicked ]: Invoked with a non-asset item.");
				return static_cast<AssetThumbnailData*>(aItem.get())->GetAssetData();
			});

		AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		for (const AssetData& assetData : assetDataView)
		{
			if (const Ref<IAssetDefinition>& pAssetDefinition = pAssetDefinitionRegistry->GetDefinitionForAsset(assetData))
			{
				const AssetHandle assetHandle = AssetManager::LoadAsset(assetData);
				if (!assetHandle.IsValid())
					continue;

				pAssetDefinition->OpenAssets({ assetHandle });
			}
		}

		ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
	}

	void AssetView::OnTileItemDoubleClicked(const SharedPtr<AssetViewItem>& aItem) noexcept
	{
		switch (aItem->GetType())
		{
		case EAssetViewItemType::Asset:
			OnAssetTileDoubleClicked(aItem);
			break;
		case EAssetViewItemType::Folder:
			OnFolderTileDoubleClick(aItem);
			break;
		default:
			RLS_ASSERT(false, "[AssetView::OnTileItemDoubleClicked]: Unknown asset view type encountered.");
			break;
		}
	}

	void AssetView::OnFilterButtonClicked() noexcept
	{
		ContextMenuBuilder contextMenuBuilder;
		
		AssetDefinitionRegistry& assetDefinitionRegistry = *Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		std::vector<Ref<IAssetDefinition>> assetDefinitions = assetDefinitionRegistry.GetAllAssetDefinitions();
		std::ranges::sort(assetDefinitions, [](const Ref<IAssetDefinition>& a, const Ref<IAssetDefinition>& b) 
			{
				return StringUtils::ToLower(a->GetAssetDisplayName()) < StringUtils::ToLower(b->GetAssetDisplayName());
			});

		contextMenuBuilder.AddSection("Common Filters")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		for (const auto& assetDefinition : assetDefinitions)
		{
			contextMenuBuilder.AddCheckBox(assetDefinition->GetAssetDisplayName())
				.Icon(assetDefinition->GetAssetIcon())
				.Value([this, type = assetDefinition->GetSupportedAssetType()]() { return m_AssetFilters.Get<AssetTypeFilter>()->IsEnabled(type); })
				.OnCheckStateChanged([this, type = assetDefinition->GetSupportedAssetType()](bool aState) { m_AssetFilters.Get<AssetTypeFilter>()->SetEnabled(type, aState); });
		}

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(contextMenuBuilder.BuildContextMenu());
	}

	void AssetView::OnFolderTileDoubleClick(const SharedPtr<AssetViewItem>& aItem) noexcept
	{
		const FolderThumbnailData* pFolderThumbnailData = static_cast<const FolderThumbnailData*>(aItem.get());
		Application::Get().SubmitToMainThread([this, path = pFolderThumbnailData->GetVirtualPath()]() 
			{
				OnEnterFolderRequested(path);
			});
	}

	Ref<ITableRow> AssetView::OnGenerateItem(const SharedPtr<AssetViewItem>& aItem) noexcept
	{
		switch (aItem->GetType())
		{
		case EAssetViewItemType::Asset:
			return CreateAssetTile(aItem);
		case EAssetViewItemType::Folder:
			return CreateFolderTile(aItem);
		default:
			RLS_ASSERT(false, "[AssetView::OnGenerateItem]: Unknown asset view type encountered.");
			break;
		}

		return nullptr;
	}

	void AssetView::OnNewFolderItemClicked(MAYBE_UNUSED const String& aParentVirtualPath) noexcept
	{
		RLS_ASSERT(false, "TODO");
	}

	void AssetView::OnPathAdded(const String& aVirtualPath, const String& aDisplayName, MAYBE_UNUSED EAssetSourceType aSourceType) noexcept
	{
		if (!m_ShowFolders)
			return;

		TextFilterExpressionEvaluator evaluator;
		evaluator.SetFilterText(m_AssetFilters.Get<AssetTextFilter>()->GetFilterText());

		if (!evaluator.TestTextFilter(aDisplayName, ETextFilterTextComparisonMode::Partial))
			return;
		if (!BelongsToCurrentView(ParentOf(aVirtualPath)))
			return;

		m_Items.push_back(MakeShared<FolderThumbnailData>(aVirtualPath, aDisplayName));
		m_pAssetsTileView->RequestRefresh();
	}

	const std::vector<SharedPtr<AssetViewItem>>* AssetView::OnRequestSource() noexcept
	{
		std::ranges::sort(m_Items, [this](SharedPtr<AssetViewItem>& aItemA, SharedPtr<AssetViewItem>& aItemB)
			{	
				if (m_SortAscending)
				{
					if (aItemA->GetType() != aItemB->GetType())
						return aItemA->GetType() > aItemB->GetType();

					return StringUtils::ToLower(aItemA->GetName()) < StringUtils::ToLower(aItemB->GetName());
				}
				else
				{
					if (aItemA->GetType() != aItemB->GetType())
						return aItemA->GetType() < aItemB->GetType();

					return StringUtils::ToLower(aItemA->GetName()) > StringUtils::ToLower(aItemB->GetName());
				}
			});

		OnRefresh();
		return &m_Items;
	}

	void AssetView::OnRender() noexcept
	{
		m_pRoot->AssignSize(GetAssignedSize());
		m_pRoot->Render();
	}

	void AssetView::OnSearchBarTextChanged(const char* aText) noexcept
	{
		m_AssetFilters.Get<AssetTextFilter>()->SetTextFilter(aText);
	}

	void AssetView::OnSelectionChangedInternal(MAYBE_UNUSED const SharedPtr<AssetViewItem>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept
	{
		OnSelectionChanged();
	}

	void AssetView::OnShowInExplorerItemClicked() noexcept
	{
		ShowSelectedInExplorer();
		ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
	}

	void AssetView::OnSortingButtonClicked() noexcept
	{
		ContextMenuBuilder contextMenuBuilder;

		contextMenuBuilder.AddSection("Sort Type")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		contextMenuBuilder.AddRadioButton("Ascending")
			.Value([this]() { return m_SortAscending; })
			.OnValueChanged([this](bool) 
				{ 
					m_SortAscending = true;
					m_pSortingButton->SetText(std::format("{} {}", ICON_FA_ARROW_DOWN_SHORT_WIDE, ICON_FA_CHEVRON_DOWN));
					m_pAssetsTileView->RequestRefresh();

					ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
				})
			.Tooltip("Sort the items in Ascending order");

		contextMenuBuilder.AddRadioButton("Descending")
			.Value([this]() { return !m_SortAscending; })
			.OnValueChanged([this](bool) 
				{
					m_SortAscending = false;
					m_pSortingButton->SetText(std::format("{} {}", ICON_FA_ARROW_UP_SHORT_WIDE, ICON_FA_CHEVRON_DOWN));
					m_pAssetsTileView->RequestRefresh();

					ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
				})
			.Tooltip("Sort the items in Descending order");

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(contextMenuBuilder.BuildContextMenu());
	}

	Reply AssetView::OnTileDragDetected(MAYBE_UNUSED AssetViewTile* aAssetViewTile) noexcept
	{
		std::vector<SharedPtr<AssetViewItem>> selectedItems;
		if (m_pAssetsTileView->GetSelectedItems(selectedItems) == 0u)
			return Reply::Unhandled();

		std::vector<AssetData> assetDatas;
		std::vector<String> paths;
		assetDatas.reserve(selectedItems.size());
		paths.reserve(selectedItems.size());

		AssetThumbnailData* pFirstAsset = nullptr;

		for (const SharedPtr<AssetViewItem>& pItem : selectedItems)
		{
			if (pItem->GetType() == EAssetViewItemType::Asset)
			{
				auto* pAsset = static_cast<AssetThumbnailData*>(pItem.get());
				assetDatas.push_back(pAsset->GetAssetData());
				
				if (!pFirstAsset)
					pFirstAsset = pAsset;
			}
			else // Path
				paths.push_back(static_cast<FolderThumbnailData*>(pItem.get())->GetVirtualPath());
		}

		// Strategy: if ANY assets are dragged they ALWAYS take prio for the preview.
		// The preview is based on the first selected asset, else the first folder.
		const bool hasAssets = pFirstAsset != nullptr;

		String previewText = hasAssets ? assetDatas.front().Name : paths.front();
		if (const auto count = selectedItems.size(); count > 1)
			previewText += std::format(" and {} other{}", count - 1, count > 2 ? "s" : "");

		Ref<Thumbnail> pThumbnail = hasAssets ? Ref<Thumbnail>{ RLS_NEW AssetThumbnail(pFirstAsset->GetWeakPtr(), Vector2(50.0f, 50.0f)) } : GetFolderThumbnail(Vector2(50.0f, 50.0f));

		return Reply::Handled().BeginDragDrop(
			RLS_NEW AssetViewDragDropOperation(assetDatas, paths, pThumbnail, previewText));
	}

	String AssetView::ParentOf(const String& aVirtualPath) const noexcept
	{
		size_t end = aVirtualPath.size();
		if (end > 1u && aVirtualPath[end - 1u] == '/')
			--end;

		const size_t lastSlash = aVirtualPath.find_last_of('/', end - 1u);

		return aVirtualPath.substr(0u, lastSlash + 1u);
	}

	bool AssetView::RequiresAssignedSize() const noexcept
	{
		return true;
	}

	void AssetView::Repopulate() noexcept
	{
		Clear();
		
		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		if (m_SourceFolders.empty())
		{
			std::vector<String> roots;

			assetRegistry.ForEachRoot([&roots](const String& aVirtualPath, const String&, EAssetSourceType)
				{
					roots.push_back(aVirtualPath);
					return true;
				});

			for (const String& root : roots)
			{
				assetRegistry.ForEachAssetWithPath(root, [this](const AssetData& aData)
					{
						OnAssetAdded(aData);
						return true;
					},
					true);
			}
		}
		else
		{
			for (const auto& aAssetData : assetRegistry.GetAssetsUnderPaths(m_SourceFolders))
				OnAssetAdded(aAssetData);

			for (const String& sourceFolder : m_SourceFolders)
			{
				assetRegistry.ForEachChildFolder(sourceFolder, [this](const String& aVirtualPath, const String& aDisplayName, EAssetSourceType aSourceType)
					{
						OnPathAdded(aVirtualPath, aDisplayName, aSourceType);
						return true;
					});
			}
		}
	}

	void AssetView::UpdateSearchBarHintText() noexcept
	{
		std::vector<String> folderNames;
		folderNames.reserve(m_SourceFolders.size());

		for (const String& virtualPath : m_SourceFolders)
			folderNames.push_back(StringUtils::Split(virtualPath, '/').back());

		const String searchBarHintText = folderNames.empty() ? "Search Assets" : std::format("Search {}", StringUtils::Join(folderNames, ", "));
		m_pSearchBar->SetHintText(searchBarHintText);
	}
}
