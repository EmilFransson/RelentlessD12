#include "AssetView.h"
#include "Assets/Factory/TextureFactory.h"

#include "Core/Editor.h"

#include "ImGui/ImGuiFonts.h"

#include "Module/ContentBrowserModule.h"

#include "Subsystem/AssetDefinitionRegistry.h"

#include "Thumbnail/AssetThumbnailPool.h"

#include "UI/DragDrop/AssetDragDropOperation.h"
#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Views/TileView.h"
#include "UI/Widgets/AssetTileItem.h"
#include "UI/Widgets/AssetTileItemTooltip.h"
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
		m_Filters.Add<AssetTextFilter>();
		m_Filters.Add<AssetTypeFilter>();
		m_Filters.Add<AssetSourceFilter>();
		m_Filters.OnFilterChanged.Connect(this, &AssetView::Repopulate);

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
		m_pTileViewBox->SetScrollBarsVisible(true);

		const Vector2 size = AssetThumbnailSizeEnumToSize(m_AssetThumbnailSize);
		
		m_pAssetsTreeView = m_pTileViewBox->AddWidget(RLS_NEW TileView<SharedPtr<AssetThumbnailData>>());

		m_pAssetsTreeView
			->SetItemWidth(size.x)
			->SetItemHeight(size.y)
			->OnRequestSource(this, &AssetView::OnRequestSource)
			->OnGenerateRow(this, &AssetView::OnGenerateItem)
			->OnSelectionChanged(this, &AssetView::OnSelectionChangedInternal)
			->OnDoubleClick(this, &AssetView::OnAssetTileItemDoubleClicked)
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

	bool AssetView::BelongsToCurrentView(const AssetData& aAssetData) const noexcept
	{
		if (m_SourceFolders.empty())
			return true;

		for (const String& src : m_SourceFolders)
		{
			if (aAssetData.PackagePath == src)
				return true;
		}

		return false;
	}

	void AssetView::Clear() noexcept
	{
		m_Items.clear();
		m_pAssetsTreeView->ClearItemsSource();
		m_pAssetsTreeView->ClearSelection();
		m_pAssetsTreeView->RequestRefresh();
	}

	AssetFilterCollection& AssetView::GetAssetFilterCollection() noexcept
	{
		return m_Filters;
	}

	EAssetThumbnailSize AssetView::GetAssetThumbnailSize() const noexcept
	{
		return m_AssetThumbnailSize;
	}

	uint32 AssetView::GetNumItems() const noexcept
	{
		return static_cast<uint32>(m_Items.size());
	}

	uint32 AssetView::GetNumSelectedItems() const noexcept
	{
		return m_pAssetsTreeView->GetNumItemsSelected();
	}

	bool AssetView::IsMainViewFocused() const noexcept
	{
		return m_pTileViewBox->IsFocused();
	}

	bool AssetView::IsMainViewHovered() const noexcept
	{
		return m_pTileViewBox->IsHovered();
	}

	void AssetView::SelectAll() noexcept
	{
		m_pAssetsTreeView->SelectAll();
	}

	void AssetView::SetAssetThumbnailSize(EAssetThumbnailSize aAssetThumbnailSize) noexcept
	{
		if (m_AssetThumbnailSize == aAssetThumbnailSize)
			return;

		m_AssetThumbnailSize = aAssetThumbnailSize;

		const Vector2 size = AssetThumbnailSizeEnumToSize(m_AssetThumbnailSize);
		m_pAssetsTreeView->SetItemWidth(size.x);
		m_pAssetsTreeView->SetItemHeight(size.y);
		Repopulate();
	}

	void AssetView::SetSourceFolders(const std::vector<String>& someVirtualPaths) noexcept
	{
		m_SourceFolders = someVirtualPaths;

		Repopulate();
		UpdateSearchBarHintText();
	}

	Vector2 AssetView::AssetThumbnailSizeEnumToSize(EAssetThumbnailSize aAssetThumbnailSize) const noexcept
	{
		switch (aAssetThumbnailSize)
		{
		case EAssetThumbnailSize::Small: return Vector2(75.0f, 127.5f);
		case EAssetThumbnailSize::Medium: return Vector2(100.0f, 170.0f);
		case EAssetThumbnailSize::Large: return Vector2(125.0f, 212.5f);
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
		if (!m_Filters.PassesAll(aAssetData))
			return;
		if (!BelongsToCurrentView(aAssetData))
			return;

		ContentBrowserModule& contentBrowser = ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		const SharedPtr<AssetThumbnailPool>& pThumbnailPool = contentBrowser.GetAssetThumbnailPool();

		m_Items.push_back(MakeShared<AssetThumbnailData>(aAssetData, pThumbnailPool));
		m_pAssetsTreeView->RequestRefresh();
	}

	Reply AssetView::OnAssetTileItemDragDetected(MAYBE_UNUSED const WidgetGeometry& aGeometry, MAYBE_UNUSED const PointerInfo& aPointerInfo) noexcept
	{
		std::vector<SharedPtr<AssetThumbnailData>> selectedItems;
		if (m_pAssetsTreeView->GetSelectedItems(selectedItems) == 0u)
			return Reply::Unhandled();

		std::vector<AssetData> assetDatas;
		assetDatas.reserve(selectedItems.size());

		for (const auto& thumbnailData : selectedItems)
			assetDatas.push_back(thumbnailData->GetAssetData());

		return Reply::Handled().BeginDragDrop(RLS_NEW AssetDragDropOperation(assetDatas));
	}

	void AssetView::OnAssetTileItemDoubleClicked(const SharedPtr<AssetThumbnailData>& aThumbnailData) noexcept
	{
		if (const Ref<IAssetDefinition>& pAssetDefinition = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>()->GetDefinitionForAsset(aThumbnailData->GetAssetData()))
		{
			const AssetHandle assetHandle = AssetManager::LoadAsset(aThumbnailData->GetAssetData());
			if (!assetHandle.IsValid())
				return;

			pAssetDefinition->OpenAssets({ assetHandle });
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
				.Value([this, type = assetDefinition->GetSupportedAssetType()]() { return m_Filters.Get<AssetTypeFilter>()->IsEnabled(type); })
				.OnCheckStateChanged([this, type = assetDefinition->GetSupportedAssetType()](bool aState) { m_Filters.Get<AssetTypeFilter>()->SetEnabled(type, aState); });
		}

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(contextMenuBuilder.BuildContextMenu());
	}

	Ref<ITableRow> AssetView::OnGenerateItem(const SharedPtr<AssetThumbnailData>& aItem) noexcept
	{
		const Vector2 size = AssetThumbnailSizeEnumToSize(m_AssetThumbnailSize);
		Ref<AssetTileItem> pAssetTileItem = RLS_NEW AssetTileItem(*aItem, Vector2(size.x, size.y), m_pAssetsTreeView);
		pAssetTileItem->OnDragDetected(this, &AssetView::OnAssetTileItemDragDetected);
		pAssetTileItem->SetTooltip(RLS_NEW AssetTileItemTooltip(aItem->GetAssetData()));

		return pAssetTileItem;
	}

	const std::vector<SharedPtr<AssetThumbnailData>>* AssetView::OnRequestSource() noexcept
	{
		std::ranges::sort(m_Items, [this](SharedPtr<AssetThumbnailData>& aAssetThumbnailDataA, SharedPtr<AssetThumbnailData>& aAssetThumbnailDataB)
			{	
				if (m_SortAscending)
					return StringUtils::ToLower(aAssetThumbnailDataA->GetAssetData().Name) < StringUtils::ToLower(aAssetThumbnailDataB->GetAssetData().Name);
				else 
					return StringUtils::ToLower(aAssetThumbnailDataA->GetAssetData().Name) > StringUtils::ToLower(aAssetThumbnailDataB->GetAssetData().Name);
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
		m_Filters.Get<AssetTextFilter>()->SetTextFilter(aText);
	}

	void AssetView::OnSelectionChangedInternal(MAYBE_UNUSED const SharedPtr<AssetThumbnailData>& aItem, MAYBE_UNUSED ESelectionType aSelectionType) noexcept
	{
		OnSelectionChanged();
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
					m_pAssetsTreeView->RequestRefresh();

					ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
				})
			.Tooltip("Sort the items in Ascending order");

		contextMenuBuilder.AddRadioButton("Descending")
			.Value([this]() { return !m_SortAscending; })
			.OnValueChanged([this](bool) 
				{
					m_SortAscending = false;
					m_pSortingButton->SetText(std::format("{} {}", ICON_FA_ARROW_UP_SHORT_WIDE, ICON_FA_CHEVRON_DOWN));
					m_pAssetsTreeView->RequestRefresh();

					ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
				})
			.Tooltip("Sort the items in Descending order");

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(contextMenuBuilder.BuildContextMenu());
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
			assetRegistry.ForEachAsset([this](const AssetData& aAssetData)
				{
					OnAssetAdded(aAssetData);
					return true;
				});
		}
		else
		{
			for (const auto& aAssetData : assetRegistry.GetAssetsUnderPaths(m_SourceFolders))
				OnAssetAdded(aAssetData);
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
