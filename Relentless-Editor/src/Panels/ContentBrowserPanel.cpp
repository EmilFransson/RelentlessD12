#include "ContentBrowserPanel.h"
#include "Core/Editor.h"

#include "ImGui/ImGuiFonts.h"

#include "Module/UIModule.h"

#include "Subsystem/AssetDefinitionRegistry.h"

#include "UI/Views/Assets/AssetView.h"
#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Views/Path/PathView.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/Separator.h"
#include "UI/Widgets/VerticalBox.h"

#include "Utility/Filter/AssetSourceFilter.h"

namespace Relentless
{
	ContentBrowserPanel::ContentBrowserPanel() noexcept
		: PanelBase(ICON_FA_FOLDER_TREE " Content Browser", ImGuiWindowFlags_None)
	{
		SetPadding(Vector2(2.0f, 0.0f));
		
		Ref<HorizontalBox> pRoot = RLS_NEW HorizontalBox();
		pRoot->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pRoot->SetPadding(FloatRect(5.0f, 5.0f, 5.0f, 5.0f));

		auto[pLeftTopBox, pLeftBottomBox, pMiddleBox, pRightTopBox, pRightMiddleBox, pRightBottomBox] = BuildLayout(pRoot);

		pLeftTopBox->AddWidget(RLS_NEW Label(std::format("{} {}", ICON_FA_CHEVRON_DOWN, Project::GetName()), UI::Fonts::Get("Medium")));
		
		m_pPathView = pLeftBottomBox->AddWidget(RLS_NEW PathView(ModuleManager::LoadModuleChecked<AssetRegistryModule>()));
		m_pPathView->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pPathView->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pPathView->OnSelectionChanged.Connect(this, &ContentBrowserPanel::OnPathViewSelectionChanged);
		m_pPathView->SetRootFilter(Callback<bool(EAssetSourceType, const String&)>::Bind(this, &ContentBrowserPanel::OnFilterRoots));

		m_pViewSeparator = pMiddleBox->AddWidget(RLS_NEW Separator(false))
			->SetActiveColor(Colors::Black)
			->SetThickness(4.0f)
			->OnMouseEnter([](Separator* aSeparator)
				{
					aSeparator->SetActiveColor(Colors::Gray);
					ModuleManager::LoadModuleChecked<UIModule>().OverrideMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);
				})
			->OnMouseExit([this](Separator* aSeparator)
				{ 
					if (!m_DraggingViewSeparator)
					{
						aSeparator->SetActiveColor(Colors::Black);
						ModuleManager::LoadModuleChecked<UIModule>().ResetMouseCursor();
					}
				});

		pRightTopBox->AddWidget(BuildToolbar());

		m_pAssetsView = pRightMiddleBox->AddWidget(RLS_NEW AssetView());
		m_pAssetsView->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pAssetsView->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pAssetsView->OnSelectionChanged.Connect(this, &ContentBrowserPanel::UpdateItemsLabel);
		m_pAssetsView->OnRefresh.Connect(this, &ContentBrowserPanel::UpdateItemsLabel);

		pRightBottomBox->AddWidget(RLS_NEW Separator())
			->SetActiveColor(Colors::Black)
			->SetThickness(3.0f);

		m_pItemsLabel = pRightBottomBox->AddWidget(RLS_NEW Label())
			->SetPadding(Vector2(0.0f, 10.0f))
			->SetMargin(FloatRect::WithLeft(10.0f));

		SetRoot(pRoot);
	}

	ContentBrowserPanel::~ContentBrowserPanel() noexcept = default;

	String ContentBrowserPanel::GetDisplayName() const noexcept
	{
		return String(ICON_FA_FOLDER_TREE " Content Browser");
	}

	String ContentBrowserPanel::GetPersistKey() const noexcept
	{
		return "Content Browser";
	}

	Ref<Button> ContentBrowserPanel::BuildAddAssetButton() noexcept
	{
		Ref<Button> pButton = RLS_NEW Button(std::format("{} Add", ICON_FA_PLUS));
		pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pButton->OnClicked(this, &ContentBrowserPanel::OnAddAssetButtonClicked);
		pButton->SetPadding(Vector2(12.0f, 6.0f));
		pButton->SetIsEnabled(false);

		m_pAddAssetButton = pButton;
		UpdateAddAssetButton();

		return pButton;
	}

	Ref<Button> ContentBrowserPanel::BuildImportAssetButton() noexcept
	{
		Ref<Button> pButton = RLS_NEW Button(std::format("{} Import", ICON_FA_DOWNLOAD));
		pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pButton->SetPadding(Vector2(12.0f, 6.0f));
		pButton->SetIsEnabled(false);
		pButton->OnClicked(this, &ContentBrowserPanel::OnImportAssetButtonClicked);

		m_pImportAssetButton = pButton;
		UpdateImportAssetButton();

		return pButton;
	}

	ContentBrowserLayout ContentBrowserPanel::BuildLayout(HorizontalBox* aRoot) noexcept
	{
		m_pPathViewBox = aRoot->AddWidget(RLS_NEW VerticalBox());
		m_pPathViewBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pPathViewBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		m_pPathViewBox->SetSize(Vector2(200.0f, -1.0f));
		m_pPathViewBox->SetSpacing(5.0f);

		VerticalBox* pLeftTopBox = m_pPathViewBox->AddWidget(RLS_NEW VerticalBox());

		VerticalBox* pLeftBottomBox = m_pPathViewBox->AddWidget(RLS_NEW VerticalBox());
		pLeftBottomBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pLeftBottomBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		VerticalBox* pMiddleBox = aRoot->AddWidget(RLS_NEW VerticalBox());
		pMiddleBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		VerticalBox* pRightBox = aRoot->AddWidget(RLS_NEW VerticalBox());
		pRightBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRightBox->SetVerticalSizePolicy(ESizePolicy::Stretch);
		pRightBox->SetMargin(FloatRect::WithLeft(10.0f));

		VerticalBox* pRightTopBox = pRightBox->AddWidget(RLS_NEW VerticalBox());
		pRightTopBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		VerticalBox* pRightMiddleBox = pRightBox->AddWidget(RLS_NEW VerticalBox());
		pRightMiddleBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRightMiddleBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		VerticalBox* pRightBottomBox = pRightBox->AddWidget(RLS_NEW VerticalBox());
		pRightBottomBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRightBottomBox->SetMargin(FloatRect(0.0f, 0.0f, 0.0f, 0.0f));
		pRightBottomBox->SetBackgroundColor(Colors::EvenRowColorDefault);

		m_pAssetsViewBox = pRightMiddleBox;

		return ContentBrowserLayout{ .LeftTopBox = pLeftTopBox, .LeftBottomBox = pLeftBottomBox, .MiddleBox = pMiddleBox, .RightTopBox = pRightTopBox, .RightMiddleBox = pRightMiddleBox, .RightBottomBox = pRightBottomBox };
	}

	Ref<HorizontalBox> ContentBrowserPanel::BuildNavigation() noexcept
	{
		Ref<HorizontalBox> pNavigationBox = RLS_NEW HorizontalBox();
		pNavigationBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pNavigateBackButton = pNavigationBox->AddWidget(Button::CreateTransparent(ICON_FA_ARROW_ROTATE_LEFT));
		m_pNavigateBackButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		m_pNavigateBackButton->SetPadding(Vector2(6.0f, 6.0f));
		m_pNavigateBackButton->OnClicked(this, &ContentBrowserPanel::OnNavigateBackButtonClicked);
		m_pNavigateBackButton->SetIsEnabled(false);

		m_pNavigateForwardButton = pNavigationBox->AddWidget(Button::CreateTransparent(ICON_FA_ARROW_ROTATE_RIGHT));
		m_pNavigateForwardButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		m_pNavigateForwardButton->SetPadding(Vector2(6.0f, 6.0f));
		m_pNavigateForwardButton->OnClicked(this, &ContentBrowserPanel::OnNavigateForwardButtonClicked);
		m_pNavigateForwardButton->SetIsEnabled(false);
		
		return pNavigationBox;
	}

	Ref<HorizontalBox> ContentBrowserPanel::BuildPathBreadcrumb() noexcept
	{
		m_pBreadcrumbBox = RLS_NEW HorizontalBox();
		m_pBreadcrumbBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pBreadcrumbBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		m_pBreadcrumbBox->SetBackgroundColor(Colors::Black);
		m_pBreadcrumbBox->SetPadding(FloatRect::WithLeft(10.0f));

		PopulateBreadcrumb();

		return m_pBreadcrumbBox;
	}

	Ref<Button> ContentBrowserPanel::BuildSettingsButton() noexcept
	{
		Ref<Button> pSettingsButton = Button::CreateTransparent(ICON_FA_GEAR);
		pSettingsButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		pSettingsButton->SetPadding(Vector2(6.0f, 6.0f));
		pSettingsButton->SetTextColor(Colors::TextInactive);
		pSettingsButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
		pSettingsButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });
		pSettingsButton->OnClicked(this, &ContentBrowserPanel::OnSettingsButtonClicked);
		pSettingsButton->SetTooltipText("Content Browser Settings.");

		return pSettingsButton;
	}

	Ref<VerticalBox> ContentBrowserPanel::BuildToolbar() noexcept
	{
		Ref<VerticalBox> pToolbarBox = RLS_NEW VerticalBox();
		pToolbarBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pToolbarBox->SetMargin(FloatRect::WithBottom(5.0f));
		pToolbarBox->SetSpacing(5.0f);
		
		Ref<HorizontalBox> pTopBox = pToolbarBox->AddWidget(RLS_NEW HorizontalBox());
		pTopBox->SetSpacing(5.0f);
		pTopBox->AddWidget(BuildAddAssetButton());
		pTopBox->AddWidget(BuildImportAssetButton());
		pTopBox->AddWidget(BuildNavigation());
		pTopBox->AddWidget(BuildPathBreadcrumb());
		pTopBox->AddWidget(BuildSettingsButton());

		return pToolbarBox;
	}

	void ContentBrowserPanel::OpenContextMenu() noexcept
	{
		const std::vector<Ref<PathListItem>> selectedItems = m_pPathView->GetSelectedItems();
		if (selectedItems.size() != 1u)
			return;

		if (selectedItems.front()->SourceType == EAssetSourceType::Engine)
			return;

		const String& virtualPath = selectedItems.front()->VirtualPath;

		AssetDefinitionRegistry* pAssetDefinitionRegistry = Editor::Get()->GetSubsystem<AssetDefinitionRegistry>();
		ContextMenuBuilder builder;

		builder.AddSection("Get")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		builder.AddItem("Import To Current Folder")
			.Tooltip("Import an asset from file to this folder.")
			.Icon(ICON_FA_DOWNLOAD)
			.OnClicked([this]()
				{
					ImportToCurrentFolder();
					ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
				});

		builder.AddSection("Create")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		for (auto& pDefinition : pAssetDefinitionRegistry->GetAllAssetDefinitions()
			| std::views::filter([](const auto& aDefinition) { return aDefinition->SupportsCreateNew(); }))
		{
			builder.AddItem(pDefinition->GetAssetDisplayName())
				.Icon(pDefinition->GetAssetIcon())
				.OnClicked([pDefinition, virtualPath]()
					{
						AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
						const String uniqueName = assetToolsModule.GenerateUniqueAssetName(virtualPath, pDefinition->GetAssetDisplayName());

						assetToolsModule.CreateAsset(pDefinition->GetSupportedAssetType(), uniqueName, virtualPath);
						ModuleManager::LoadModuleChecked<UIModule>().DestroyActiveContextMenu();
					});
		}

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

	void ContentBrowserPanel::ImportToCurrentFolder() noexcept
	{
		std::vector<Ref<PathListItem>> selectedItems = m_pPathView->GetSelectedItems();

		const std::vector<Path> paths = Platform::OpenFileDialog();
		if (paths.empty())
			return;

		AssetRegistryModule& assetRegistryModule = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		std::vector<AssetImportTask> importTasks;
		importTasks.reserve(paths.size());

		for (const Path& path : paths)
		{
			AssetImportTask& importTask = importTasks.emplace_back();
			importTask.FilePath = path;
			importTask.DestinationPath = assetRegistryModule.VirtualPathToAbsolutePath(selectedItems.front()->VirtualPath);
		}

		AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		assetToolsModule.ImportAsync(importTasks, [](const std::vector<AssetImportResult>&) {});
	}

	void ContentBrowserPanel::OnAddAssetButtonClicked() noexcept
	{
		OpenContextMenu();
	}

	bool ContentBrowserPanel::OnFilterRoots(EAssetSourceType aAssetSourceType, MAYBE_UNUSED const String& aVirtualPath) noexcept
	{
		return m_ShowEngineContent || aAssetSourceType != EAssetSourceType::Engine;
	}

	void ContentBrowserPanel::OnImportAssetButtonClicked() noexcept
	{
		ImportToCurrentFolder();
	}

	bool ContentBrowserPanel::AcceptsMouseInput() const noexcept
	{
		if (m_pViewSeparator->IsHovered())
			return true;

		if (m_DraggingViewSeparator)
			return true;

		if (m_pAssetsView->IsMainViewHovered())
			return true;

		return false;
	}

	bool ContentBrowserPanel::OnKeyPressedEvent(KeyPressedEvent& aKeyPressedEvent) noexcept
	{
		switch (aKeyPressedEvent.key)
		{
		case RLS_Key::A:
		{
			if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
			{
				if (m_pAssetsView->IsMainViewFocused())
				{
					m_pAssetsView->SelectAll();
					return true;
				}
				else if (m_pPathView->IsMainViewFocused())
				{
					m_pPathView->SelectAll();
					return true;
				}
			}
			break;
		}
		default:
			break;
		}

		return false;
	}

	bool ContentBrowserPanel::OnLeftMouseButtonPressedEvent(MAYBE_UNUSED LeftMouseButtonPressedEvent& aLeftMouseButtonPressedEvent) noexcept
	{
		if (m_pViewSeparator->IsHovered())
		{
			m_DragStartWidth = m_pPathViewBox->GetFixedSize().x;
			m_DragStartMouseX = ImGui::GetMousePos().x;

			m_DraggingViewSeparator = true;
			return true;
		}

		return false;
	}

	bool ContentBrowserPanel::OnLeftMouseButtonReleasedEvent(MAYBE_UNUSED LeftMouseButtonReleasedEvent& aLeftMouseButtonReleasedEvent) noexcept
	{
		if (m_DraggingViewSeparator)
		{
			m_DraggingViewSeparator = false;

			if (!m_pViewSeparator->IsHovered())
			{
				m_pViewSeparator->SetActiveColor(Colors::Black);
				ModuleManager::LoadModuleChecked<UIModule>().ResetMouseCursor();
			}
		}
		
		return true;
	}

	bool ContentBrowserPanel::OnMouseDragEvent(MAYBE_UNUSED MouseDragEvent& aMouseDragEvent) noexcept
	{
		if (!m_DraggingViewSeparator)
			return false;

		const Vector2& size = m_pPathViewBox->GetFixedSize();
		const Vector2u& panelSize = GetContentRegionAvail();
		const float mouseDeltaX = static_cast<float>(Mouse::GetCursorScreenPosition().x) - m_DragStartMouseX;
		
		const float target = Math::Min(Math::Max(2.0f, m_DragStartWidth + mouseDeltaX), panelSize.x - 20.f);
		m_pPathViewBox->SetSize(Vector2(target, size.y));
		
		return true;
	}

	bool ContentBrowserPanel::OnRightMouseButtonReleasedEvent(MAYBE_UNUSED RightMouseButtonReleasedEvent& aRightMouseButtonReleasedEvent) noexcept
	{
		if (!m_pAssetsView->IsMainViewHovered())
			return false;

		if (m_pAssetsView->GetNumSelectedItems() != 0u)
			return false;

		OpenContextMenu();

		return true;
	}

	void ContentBrowserPanel::OnNavigateBackButtonClicked() noexcept
	{
		if (m_BreadcrumbBackwardStack.empty())
			return;

		m_BreadcrumbForwardStack.push_back(m_CurrentPath);
		m_CurrentPath = m_BreadcrumbBackwardStack.back();
		m_BreadcrumbBackwardStack.pop_back();

		ScopedSuspend suspendHistory(m_SuppressHistoryRecording);
		m_pPathView->SetSelectedItemByVirtualPath(m_CurrentPath);
		
		UpdateNavigationButtons();
	}

	void ContentBrowserPanel::OnNavigateForwardButtonClicked() noexcept
	{
		if (m_BreadcrumbForwardStack.empty())
			return;

		m_BreadcrumbBackwardStack.push_back(m_CurrentPath);
		m_CurrentPath = m_BreadcrumbForwardStack.back();
		m_BreadcrumbForwardStack.pop_back();

		ScopedSuspend suspendHistory(m_SuppressHistoryRecording);
		m_pPathView->SetSelectedItemByVirtualPath(m_CurrentPath);
		
		UpdateNavigationButtons();
	}

	void ContentBrowserPanel::OnPathViewSelectionChanged() noexcept
	{
		std::vector<Ref<PathListItem>> selectedItems = m_pPathView->GetSelectedItems();
		
		std::vector<String> paths;
		paths.reserve(selectedItems.size());

		for (const Ref<PathListItem>& pItem : selectedItems)
			paths.push_back(pItem->VirtualPath);

		m_pAssetsView->SetSourceFolders(paths);

		if (!m_SuppressHistoryRecording)
		{
			RecordHistory(paths);
			UpdateNavigationButtons();
		}

		UpdateAddAssetButton();
		UpdateImportAssetButton();
		PopulateBreadcrumb();
	}

	void ContentBrowserPanel::OnSettingsButtonClicked() noexcept
	{
		ContextMenuBuilder builder;

		builder.AddSection("Show")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		builder.AddCheckBox("Engine Content")
			.Tooltip("Show engine content in the view.")
			.Value([this]() { return m_ShowEngineContent; })
			.OnCheckStateChanged([this](bool aState) 
				{ 
					m_ShowEngineContent = aState;
					m_pPathView->Refresh();
					m_pAssetsView->GetAssetFilterCollection().Get<AssetSourceFilter>()->SetSourceVisible(EAssetSourceType::Engine, aState);
				});

		builder.AddSection("Thumbnails")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		builder.AddSubmenu("Thumbnail Size")
			.OnOpen([this](ContextMenuBuilder& aBuilder) 
				{
					aBuilder.AddRadioButton("Small")
						.Value([this]() { return m_pAssetsView->GetAssetThumbnailSize() == EAssetThumbnailSize::Small; })
						.OnValueChanged([this](bool) { m_pAssetsView->SetAssetThumbnailSize(EAssetThumbnailSize::Small); });

					aBuilder.AddRadioButton("Medium")
						.Value([this]() { return m_pAssetsView->GetAssetThumbnailSize() == EAssetThumbnailSize::Medium; })
						.OnValueChanged([this](bool) { m_pAssetsView->SetAssetThumbnailSize(EAssetThumbnailSize::Medium); });

					aBuilder.AddRadioButton("Large")
						.Value([this]() { return m_pAssetsView->GetAssetThumbnailSize() == EAssetThumbnailSize::Large; })
						.OnValueChanged([this](bool) { m_pAssetsView->SetAssetThumbnailSize(EAssetThumbnailSize::Large); });
				});

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

	void ContentBrowserPanel::PopulateBreadcrumb() noexcept
	{
		m_pBreadcrumbBox->RemoveAllWidgets();

		auto CreateCrumbButton = [](StringView aText, Callback<void()>&& aOnClickCallback) -> Ref<Button>
			{
				Ref<Button> pButton = Button::CreateTransparent(aText);
				pButton->OnClicked([callBack = std::move(aOnClickCallback)]() { callBack(); });
				pButton->SetTextColor(Colors::TextInactive);
				pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
				pButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });
				pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

				return pButton;
			};

		if (!m_pPathView || m_pPathView->GetSelectedItems().empty())
		{
			m_pBreadcrumbBox->AddWidget(CreateCrumbButton("All Assets", [](){}));
			m_pBreadcrumbBox->AddWidget(CreateCrumbButton(ICON_FA_CHEVRON_RIGHT, [](){}));
		}
		else
		{
			std::vector<Ref<PathListItem>> selectedItems = m_pPathView->GetSelectedItems();
			if (selectedItems.size() == 1u)
			{
				const Ref<PathListItem>& pItem = selectedItems.front();
				const std::vector<String> crumbs = StringUtils::Split(pItem->VirtualPath, '/');

				String accumulator = "/";
				for (uint32 i = 0u; i < crumbs.size(); ++i)
				{
					if (crumbs[i].empty()) 
						continue;
					
					accumulator += crumbs[i] + "/";
					const String crumbPath = accumulator;

					m_pBreadcrumbBox->AddWidget(CreateCrumbButton(crumbs[i], [this, crumbPath]()
						{
							Application::Get().SubmitToMainThread([this, crumbPath]() { m_pPathView->SetSelectedItemByVirtualPath(crumbPath); });
						}));

					if (i != crumbs.size() - 1u || pItem->IsRoot)
						m_pBreadcrumbBox->AddWidget(CreateCrumbButton(ICON_FA_CHEVRON_RIGHT, []() {}));
				}
			}
			else
			{
				m_pBreadcrumbBox->AddWidget(CreateCrumbButton("Multiple Folders", []() {}));
				m_pBreadcrumbBox->AddWidget(CreateCrumbButton(ICON_FA_CHEVRON_RIGHT, []() {}));
			}
		}
	}

	void ContentBrowserPanel::RecordHistory(const std::vector<String>& somePaths) noexcept
	{
		if (!m_CurrentPath.empty())
			m_BreadcrumbBackwardStack.push_back(m_CurrentPath);
		
		m_BreadcrumbForwardStack.clear();
		m_CurrentPath = somePaths.empty() ? String() : somePaths.back();
	}

	void ContentBrowserPanel::UpdateAddAssetButton() noexcept
	{
		if (!m_pPathView)
			m_pAddAssetButton->SetIsEnabled(false);
		else
		{
			std::vector<Ref<PathListItem>> items = m_pPathView->GetSelectedItems();
			if (items.empty())
			{
				m_pAddAssetButton->SetIsEnabled(false);
				m_pAddAssetButton->SetTooltipText("No path is selected as an add target.");
			}
			else if (items.size() > 1u)
			{
				m_pAddAssetButton->SetIsEnabled(false);
				m_pAddAssetButton->SetTooltipText("Cannot add content to multiple paths.");
			}
			else if (items.front()->SourceType == EAssetSourceType::Engine)
			{
				m_pAddAssetButton->SetIsEnabled(false);
				m_pAddAssetButton->SetTooltipText("Cannot add content to Engine paths.");
			}
			else
			{
				m_pAddAssetButton->SetIsEnabled(true);
				m_pAddAssetButton->SetTooltipText(std::format("Create new content in {}", items.front()->VirtualPath));
			}
		}
	}

	void ContentBrowserPanel::UpdateImportAssetButton() noexcept
	{
		if (!m_pPathView)
			m_pImportAssetButton->SetIsEnabled(false);
		else
		{
			std::vector<Ref<PathListItem>> items = m_pPathView->GetSelectedItems();
			if (items.empty())
			{
				m_pImportAssetButton->SetIsEnabled(false);
				m_pImportAssetButton->SetTooltipText("No path is selected as an import target.");
			}
			else if (items.size() > 1u)
			{
				m_pImportAssetButton->SetIsEnabled(false);
				m_pImportAssetButton->SetTooltipText("Cannot import content to multiple paths.");
			}
			else if (items.front()->SourceType == EAssetSourceType::Engine)
			{
				m_pImportAssetButton->SetIsEnabled(false);
				m_pImportAssetButton->SetTooltipText("Cannot import content to Engine paths.");
			}
			else
			{
				m_pImportAssetButton->SetIsEnabled(true);
				m_pImportAssetButton->SetTooltipText(std::format("Import new content to {}", items.front()->VirtualPath));
			}
		}
	}

	void ContentBrowserPanel::UpdateItemsLabel() noexcept
	{
		const uint32 numItems = m_pAssetsView->GetNumItems();
		const uint32 numSelectedItems = m_pAssetsView->GetNumSelectedItems();
		m_pItemsLabel->SetText(std::format("{} item{} {}", numItems, numItems != 1 ? "s" : "", numSelectedItems > 0 ? std::format("({} selected)", numSelectedItems) : ""));
	}

	void ContentBrowserPanel::UpdateNavigationButtons() noexcept
	{
		const String backTooltipText = m_BreadcrumbBackwardStack.empty() ? "" : std::format("Back to {}", m_BreadcrumbBackwardStack.back());
		m_pNavigateBackButton->SetTooltipText(backTooltipText);
		m_pNavigateBackButton->SetIsEnabled(!m_BreadcrumbBackwardStack.empty());

		const String forwardTooltipText = m_BreadcrumbForwardStack.empty() ? "" : std::format("Forward to {}", m_BreadcrumbForwardStack.back());
		m_pNavigateForwardButton->SetTooltipText(forwardTooltipText);
		m_pNavigateForwardButton->SetIsEnabled(!m_BreadcrumbForwardStack.empty());
	}
}
