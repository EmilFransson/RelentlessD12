#include "EntityOutlinerView.h"

#include "../../../Core/Editor.h"

namespace Relentless
{
	#define RLS_SCOPED_SUSPEND() ScopedFlag _suspend{ m_SuspendNotifications }

	EntityOutlinerView::EntityOutlinerView() noexcept
	{
		Editor* pEditor = Editor::Get();

		pEditor->OnPreSceneChanged.Connect(this, &EntityOutlinerView::OnPreSceneChanged);
		pEditor->OnSceneChanged.Connect(this, &EntityOutlinerView::OnSceneChanged);

		const UniquePtr<Selection>& pSelection = pEditor->GetSelection();
		pSelection->OnSelectionChanged.Connect(this, &EntityOutlinerView::OnSelectionChangedExternally);

		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();
		
		pFoldersManager->OnEntityFolderCreated.Connect(this, &EntityOutlinerView::OnFolderCreated);
		pFoldersManager->OnEntityFolderDelete.Connect(this, &EntityOutlinerView::OnEntityFolderDeleted);
		pFoldersManager->OnEntityFolderMoved.Connect(this, &EntityOutlinerView::OnFolderMoved);
		pFoldersManager->OnEntityAttachedToFolder.Connect(this, &EntityOutlinerView::OnEntityAttachedToFolder);
		pFoldersManager->OnEntityRemovedFromFolder.Connect(this, &EntityOutlinerView::OnEntityRemovedFromFolder);

		std::shared_ptr<HeaderRow> pHeaderRow = std::make_shared<HeaderRow>();
		pHeaderRow->SetIsPinned(true);

		{
			Column column;
			column.pLabel = new Label(ICON_FA_EYE);
			column.pLabel
				->SetAlpha(0.7f)
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetTooltipText("Visibility");

			column.Flags = ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;
			column.Weight = 22.0f;

			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.pLabel = new Label("Item Label");
			column.pLabel
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetTooltipText("Item Label");

			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.pLabel = new Label("Type");
			column.pLabel
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetTooltipText("Type");

			pHeaderRow->AddColumn(column);
		}

		m_pOutlinerTreeView = new TreeView<Ref<OutlinerListItem>>(pHeaderRow);
		m_pOutlinerTreeView->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		m_pOutlinerTreeView->SetFlags(ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Sortable | 
			ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY);

		m_pOutlinerTreeView
			->OnGetChildren(this, &EntityOutlinerView::OnGetChildren)
			->OnRequestSource(this, &EntityOutlinerView::OnRequestSource)
			->OnGenerateRow(this, &EntityOutlinerView::OnGenerateRow)
			->OnDoubleClick(this, &EntityOutlinerView::OnRowDoubleClicked)
			->OnDebugItemToString(this, &EntityOutlinerView::OnDebugItemToString)
			->OnSelectionChanged(this, &EntityOutlinerView::OnSelectionChanged)
			->OnContextMenuOpening(this, &EntityOutlinerView::OnContextMenuOpening)
			->OnItemScrolledIntoView(this, &EntityOutlinerView::OnItemScrolledIntoView);

		m_pOutlinerTreeView->OnTreeRefreshed.Connect(this, &EntityOutlinerView::OnOutlinerTreeRefreshed);

		m_pMainBox = new VerticalBoxEx();
		
		Ref<VerticalBoxEx> pTopVerticalBox = new VerticalBoxEx();
		Ref<VerticalBoxEx> pMiddleVerticalBox = new VerticalBoxEx({}, true);
		pMiddleVerticalBox->SetSizePolicy(ESizePolicy::Stretch);

		Ref<HorizontalBoxEx> pHorizontalBox = new HorizontalBoxEx();
		Ref<HorizontalBoxEx> pLeftBox = new HorizontalBoxEx({}, true);
		pLeftBox->SetSizePolicy(ESizePolicy::Stretch);
		Ref<HorizontalBoxEx> pRightBox = new HorizontalBoxEx({}, true);

		pHorizontalBox->SetMargin(FloatRect(0.0f, 5.0f, 0.0f, 5.0f));

		pLeftBox->AddWidget(new SearchBar("Search...", true))
			->OnTextChanged(this, &EntityOutlinerView::OnSearchTextChanged)
			->OnTextCommitted(this, &EntityOutlinerView::OnSearchTextCommitted)
			->SetSizePolicy(ESizePolicy::Stretch);

		pRightBox->AddWidget(new Button(ICON_FA_FOLDER_PLUS))
			->OnClicked(this, &EntityOutlinerView::OnCreateNewFolderButtonClicked)
			->SetFont(ImGui::GetIO().Fonts->Fonts[2])
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Gray)
			->SetBorderColor(Colors::Transparent)
			->SetTooltipText("Create a new folder containing the current selection");

		pRightBox->AddWidget(new Button(ICON_FA_GEAR))
			->SetFont(ImGui::GetIO().Fonts->Fonts[2])
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Gray)
			->SetBorderColor(Colors::Transparent);

		m_pOutlinerListBox = new HorizontalBoxEx(Vector2(0.0f, 0.0f), true);
		m_pOutlinerListBox->SetSizePolicy(ESizePolicy::Stretch);
		m_pOutlinerListBox->OnFocusChanged.Connect(this, &EntityOutlinerView::OnFocusChanged);
		m_pOutlinerListBox->AddWidget(m_pOutlinerTreeView);

		pHorizontalBox->AddWidget(pLeftBox);
		pHorizontalBox->AddWidget(pRightBox);

		pTopVerticalBox->AddWidget(pHorizontalBox);
		pMiddleVerticalBox->AddWidget(m_pOutlinerListBox);

		m_pMainBox->AddWidget(pTopVerticalBox);
		m_pMainBox->AddWidget(pMiddleVerticalBox);

		Ref<VerticalBoxEx> pBorderBox = new VerticalBoxEx();
		pBorderBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Bottom);

		Border* pBorder = pBorderBox->AddWidget(new Border());
		HorizontalBoxEx* pInBox = pBorder->SetContent(new HorizontalBoxEx({}, true));
		pInBox->SetEnableScrolling(false);
		pInBox->SetSizePolicy(ESizePolicy::Stretch);

		pInBox->AddWidget(new Label(""))
			->SetFont(ImGui::GetIO().Fonts->Fonts[2])
			->SetPadding(Vector2(10.0f, 10.0f));

		m_pMainBox->AddWidget(pBorderBox);

		m_pFilter = std::make_unique<TextFilterExpressionEvaluator>();
		m_pPolicies = std::make_unique<EntityOutlinerPolicies>();

		if (Scene* pScene = pEditor->GetActiveScene())
			OnSceneChanged(pScene);
	}

	EntityOutlinerView::~EntityOutlinerView() noexcept
	{
		Editor* pEditor = Editor::Get();
		if (!pEditor)
			return;

		pEditor->OnShutDown.Detach(this);
		pEditor->OnPreSceneChanged.Detach(this);
		pEditor->OnSceneChanged.Detach(this);

		if (const UniquePtr<Selection>& pSelection = pEditor->GetSelection())
			pSelection->OnSelectionChanged.Detach(this);

		if (const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager())
		{
			pFoldersManager->OnEntityFolderCreated.Detach(this);
			pFoldersManager->OnEntityFolderDeleted.Detach(this);
			pFoldersManager->OnEntityFolderMoved.Detach(this);
			pFoldersManager->OnEntityAttachedToFolder.Detach(this);
			pFoldersManager->OnEntityRemovedFromFolder.Detach(this);
		}

		if (Scene* pScene = pEditor->GetActiveScene())
		{
			pScene->OnEntityCreated.Detach(this);
			pScene->OnEntityPreDestroyed.Detach(this);
			pScene->OnEntityDestroyed.Detach(this);
			pScene->OnEntityAttached.Detach(this);
			pScene->OnEntityDetached.Detach(this);
			pScene->OnEntityVisibilityChanged.Detach(this);
		}
	}

	void EntityOutlinerView::DuplicateSelection() noexcept
	{
		return;

		std::vector<Ref<OutlinerListItem>> selectedItems;
		if (m_pOutlinerTreeView->GetSelectedItems(selectedItems) == 0)
			return;

		std::vector<entity> selectedEntities;
		std::vector<EntityFolder*> selectedFolders;

		for (const auto& pSelectedItem : selectedItems)
		{
			if (pSelectedItem->IsEntity())
				selectedEntities.push_back(pSelectedItem->AsEntity());
			else if (pSelectedItem->IsFolder())
				selectedFolders.push_back(pSelectedItem->AsFolder());
		}

		if (selectedEntities.empty() && selectedFolders.empty())
			return;
	}

	Ref<OutlinerListItem> EntityOutlinerView::CreateEntityListItem(entity aEntity) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return nullptr;

		Ref<OutlinerListItem> pEntityItem = new OutlinerListItem();
		pEntityItem->Payload = aEntity;

		m_EntityToItemMap[pEditor->GetActiveScene()->GetEntityManager().Get<IDComponent>(aEntity).UuId] = pEntityItem;
		m_pItemToScrollIntoView = pEntityItem;

		return pEntityItem;
	}

	Ref<OutlinerListItem> EntityOutlinerView::CreateEntityFolderListItem(EntityFolder* apFolder) noexcept
	{
		Ref<OutlinerListItem> pFolderListItem = new OutlinerListItem();
		pFolderListItem->Payload = apFolder;
		
		m_FolderToItemMap[apFolder->GetUUID()] = pFolderListItem;
		m_pItemToScrollIntoView = pFolderListItem;

		return pFolderListItem;
	}

	Ref<OutlinerListItem> EntityOutlinerView::CreateSceneListItem(Scene* pScene) noexcept
	{
		Ref<OutlinerListItem> pSceneListItem = new OutlinerListItem();
		pSceneListItem->Payload = pScene;

		m_pItemToScrollIntoView = pSceneListItem;

		return pSceneListItem;
	}

	void EntityOutlinerView::DeselectAllFolders() noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		if (m_SelectedFolders.empty())
			return;

		std::vector<UUID> selectedFolderUUIDS(m_SelectedFolders.begin(), m_SelectedFolders.end());

		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();

		pFoldersManager->ForEachFolderWithRootObject(FolderRoot::CreateFromScene(*pEditor->GetActiveScene()), [&](const EntityFolder& aFolder)
			{
				if (std::ranges::any_of(selectedFolderUUIDS, [&aFolder](const UUID& aUUID) { return aFolder.GetUUID() == aUUID; }))
				{
					const Ref<OutlinerListItem>& pFolderItem = GetFolderItem(pFoldersManager->GetFolder(*pEditor->GetActiveScene(), aFolder.GetPath()));
					OnSelectionChanged(pFolderItem, ESelectionType::Deselected);
					m_pOutlinerTreeView->SetItemSelection(pFolderItem, ESelectionType::Deselected);
				}

				return true;
			});
	}

	void EntityOutlinerView::ExecuteMovePlan(const EntityOutlinerPolicies::MovePlan& aMovePlan, Scene& aScene, const OutlinerPayload& targetPayload) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();
		
		const bool targetIsEntity = std::holds_alternative<entity>(targetPayload);
		const bool targetIsFolder = std::holds_alternative<EntityFolder*>(targetPayload);
		const bool targetIsScene = std::holds_alternative<Scene*>(targetPayload);

		for (const EntityOutlinerPolicies::ItemMoveResolution& itemResolution : aMovePlan.ResolvedItems)
		{
			const bool resolvedIsEntity = std::holds_alternative<entity>(itemResolution.Item);
			const bool resolvedIsFolder = std::holds_alternative<EntityFolder*>(itemResolution.Item);

			const entity resolvedEntity = resolvedIsEntity ? std::get<entity>(itemResolution.Item) : NULL_ENTITY;
			const EntityFolder* resolvedFolder = resolvedIsFolder ? std::get<EntityFolder*>(itemResolution.Item) : nullptr;

			switch (itemResolution.MoveOperation)
			{
			case EMoveOperation::NoOp:
				break;
			case EMoveOperation::AttachToTarget:
			{
				if (resolvedIsEntity)
				{
					pFoldersManager->RemoveEntityFromCurrentFolder(aScene, resolvedEntity);

					if (targetIsEntity)
						aScene.AttachEntity(resolvedEntity, std::get<entity>(targetPayload));
					else if (targetIsFolder)
					{
						pFoldersManager->RemoveEntityFromCurrentFolder(aScene, resolvedEntity);
						aScene.DetachEntity(resolvedEntity);

						const Folder folder = Folder(FolderRoot::CreateFromScene(aScene), std::get<EntityFolder*>(targetPayload)->GetPath());
						pFoldersManager->AttachEntityToFolder(aScene, resolvedEntity, folder);
					}
					else
					{
						pFoldersManager->RemoveEntityFromCurrentFolder(aScene, resolvedEntity);
						aScene.DetachEntity(resolvedEntity);
					}
				}
				else if (resolvedIsFolder)
				{
					if (targetIsFolder)
						pFoldersManager->RenameFolder(aScene, resolvedFolder->GetPath(), FilepathUtils::CombineDisplay(std::get<EntityFolder*>(targetPayload)->GetPath(), resolvedFolder->GetLabel()));
					else 
						pFoldersManager->RenameFolder(aScene, resolvedFolder->GetPath(), resolvedFolder->GetLabel());
				}
				break;
			}
			case EMoveOperation::DetachFromTarget:
			{
				if (resolvedIsEntity)
				{
					if (targetIsEntity)
						aScene.DetachEntity(resolvedEntity);
				}
				break;
			}
			case EMoveOperation::ReattachToParentOfTarget:
			{
				if (resolvedIsEntity)
				{
					if (targetIsEntity)
					{
						aScene.DetachEntity(resolvedEntity);
						pFoldersManager->RemoveEntityFromCurrentFolder(aScene, resolvedEntity);

						const Folder folder = aScene.GetEntityManager().Get<FolderComponent>(std::get<entity>(targetPayload)).Folder;
						pFoldersManager->AttachEntityToFolder(aScene, resolvedEntity, folder);
					}
				}
				break;
			}
			}
		}
	}

	const String& EntityOutlinerView::GetItemName(const Ref<OutlinerListItem>& pItem) const noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return String();

		if (pItem->IsEntity())
			return pEditor->GetActiveScene()->GetEntityManager().Get<NameComponent>(pItem->AsEntity()).Name;
		else if (pItem->IsFolder())
			return pItem->AsFolder()->GetLabel();
		else
			return pItem->AsScene()->GetName();
	}

	const String& EntityOutlinerView::GetRowName(const OutlinerTableRow* pRow) const noexcept
	{
		return GetItemName(m_pOutlinerTreeView->GetItemFromWidget(pRow));
	}

	const Ref<OutlinerListItem>& EntityOutlinerView::GetEntityItem(entity aEntity) const noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return nullptr;

		return m_EntityToItemMap.at(pEditor->GetActiveScene()->GetEntityManager().Get<IDComponent>(aEntity).UuId);
	}

	const Ref<OutlinerListItem>& EntityOutlinerView::GetFolderItem(EntityFolder* pAFolder) const noexcept
	{
		return m_FolderToItemMap.at(pAFolder->GetUUID());
	}

	Ref<OutlinerListItem> EntityOutlinerView::GetRootSceneItem() const noexcept
	{
		RLS_ASSERT(m_ListItems.size() == 1 && m_ListItems.at(0)->IsScene(), "[EntityOutlinerView::GetRootSceneItem]: No scene root item in list.");
		return m_ListItems.at(0);
	}

	Ref<ContextMenu> EntityOutlinerView::OnContextMenuOpening(const Ref<OutlinerListItem>& item) noexcept
	{
		Ref<MenuBuilder> pBuilder = new MenuBuilder();

		Ref<ContextMenu> pMenu = pBuilder
			->AddSection("ENTITY OPTIONS")
				->BeginSubMenu("Edit", "Edit Current Selection", ICON_FA_PEN_RULER)
					->AddMenuEntry("Duplicate", "Duplicate Selection", ICON_FA_COPY, this, &EntityOutlinerView::OnDuplicateSelection)
					->AddMenuEntry("Delete", "Delete Current Selection", ICON_FA_DELETE_LEFT, this, &EntityOutlinerView::OnDeleteSelection)
					->AddMenuEntry("Rename", "Rename Current Selection", ICON_FA_PEN, this, &EntityOutlinerView::OnRenameSelection, m_pOutlinerTreeView->GetNumItemsSelected() == 1)
				->EndSubMenu()
			->AddSection("VIEW OPTIONS")
			->Build();

		pMenu->SetSpacing(Vector2(8.0f, 1.0f));
		return pMenu;
	}

	void EntityOutlinerView::OnCreateNewFolderButtonClicked() noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();
		Scene& scene = *pEditor->GetActiveScene();

		std::vector<Ref<OutlinerListItem>> selectedItems;
		m_pOutlinerTreeView->GetSelectedItems(selectedItems);

		std::vector<EntityFolder*> folders;
		std::vector<entity> entities;

		for (uint32 i = 0u; i < selectedItems.size(); ++i)
		{
			if (selectedItems[i]->IsFolder())
				folders.push_back(selectedItems[i]->AsFolder());
			else if (selectedItems[i]->IsEntity())
				entities.push_back(selectedItems[i]->AsEntity());
		}

		EntityFolder* pNewFolder = nullptr;

		if (folders.size() == 1)
		{
			const String newFolderName = pFoldersManager->GetDefaultFolderName(scene, folders.front()->GetPath());
			pNewFolder = pFoldersManager->CreateFolder(scene, FilepathUtils::CombineDisplay(folders.front()->GetPath(), newFolderName));
			if (!pNewFolder)
				return;

			EntityOutlinerPolicies::MoveContext context
			{
				.Scene = scene,
				.FoldersManager = *pFoldersManager,
				.TargetPayload = pNewFolder,
				.Entities = entities
			};

			const EntityOutlinerPolicies::MovePlan movePlan = m_pPolicies->ResolveMoveRequest(context);
			ExecuteMovePlan(movePlan, scene, pNewFolder);
		}
		else
		{
			std::vector<EntityFolder*> prunedFolders = MergeFoldersByLabel(folders);

			pNewFolder = pFoldersManager->CreateFolderContainingSelection(scene);
			m_pOutlinerTreeView->SetItemSelection(GetFolderItem(pNewFolder), ESelectionType::Selected);
			m_pFolderToRenameWhenScrolledIntoView = GetFolderItem(pNewFolder);

			if (!pNewFolder)
				return;

			if (!selectedItems.empty())
			{
				EntityOutlinerPolicies::MoveContext context
				{
					.Scene = scene,
					.FoldersManager = *pFoldersManager,
					.TargetPayload = pNewFolder,
					.Entities = entities,
					.Folders = prunedFolders
				};

				const EntityOutlinerPolicies::MovePlan movePlan = m_pPolicies->ResolveMoveRequest(context);
				ExecuteMovePlan(movePlan, scene, pNewFolder);
			}
		}

		RecreateItemHierarchy();

		m_pOutlinerTreeView->ClearSelection();
		m_pOutlinerTreeView->SetItemSelection(GetFolderItem(pNewFolder), ESelectionType::Selected);
		m_pFolderToRenameWhenScrolledIntoView = GetFolderItem(pNewFolder);
	}

	void EntityOutlinerView::OnDeleteSelection() noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		std::vector<Ref<OutlinerListItem>> selectedItems;
		if (m_pOutlinerTreeView->GetSelectedItems(selectedItems) == 0)
			return;

		Scene& scene = *pEditor->GetActiveScene();
		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();

		for (const Ref<OutlinerListItem>& pItem : selectedItems)
		{
			if (pItem->IsEntity())
				scene.DestroyEntity(pItem->AsEntity());
			else if (pItem->IsFolder())
				pFoldersManager->DeleteFolder(scene, pItem->AsFolder()->GetPath());
		}
	}

	void EntityOutlinerView::OnDuplicateSelection() noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		//TODO: Consider deferring to Editor

		std::vector<Ref<OutlinerListItem>> selectedItems;
		if (m_pOutlinerTreeView->GetSelectedItems(selectedItems) == 0)
			return;

		Scene* pScene = pEditor->GetActiveScene();
		const UniquePtr<Selection>& pSelection = pEditor->GetSelection();

		pSelection->DeselectAllEntities();

		std::unordered_set<entity> roots;

		for (size_t i = 0u; i < selectedItems.size(); ++i)
		{
			RLS_ASSERT(selectedItems[i]->IsEntity(), "UNIMPLEMENTED!");

			if (!std::any_of(selectedItems.begin(), selectedItems.end(), [&](const Ref<OutlinerListItem>& pItem)
				{
					return pItem->IsEntity() && pScene->EntityIsParent(selectedItems[i]->AsEntity(), pItem->AsEntity());
				}))
			{
				roots.insert(selectedItems[i]->AsEntity());
				//TODO: Erase root list items.
			}
		}

		auto&& RecurseDuplicate = [&](auto&& Self, entity toDuplicate, entity parent = NULL_ENTITY) -> void
			{
				const entity duplicatedEntity = pEditor->GetActiveScene()->DuplicateEntity(toDuplicate, false);
				pSelection->SelectEntity(duplicatedEntity);

				if (parent != NULL_ENTITY)
					pScene->AttachEntity(duplicatedEntity, parent);

				for (const Ref<OutlinerListItem>& pItem : selectedItems)
				{
					if (pScene->EntityIsChild(pItem->AsEntity(), toDuplicate))
						Self(Self, pItem->AsEntity(), duplicatedEntity);
				}
			};

		for (entity e : roots)
			RecurseDuplicate(RecurseDuplicate, e);
	}

	String EntityOutlinerView::OnDebugItemToString(const Ref<OutlinerListItem>& item) const noexcept
	{
		if (item->IsEntity())
			return std::format("Entity: {}", item->AsEntity());
		else if (item->IsFolder())
			return std::format("Folder: {}", item->AsFolder()->GetLabel());
		else 
			return std::format("Scene: {}", item->AsScene()->GetName());
	}

	Ref<DragDropOperation> EntityOutlinerView::OnDragDetected(OutlinerTableRow* pRow) noexcept
	{
		Ref<OutlinerDragDropOperation> pEntityDragOp = new OutlinerDragDropOperation(pRow);

		std::vector<Ref<OutlinerListItem>> selectedItems;
		m_pOutlinerTreeView->GetSelectedItems(selectedItems);

		std::vector<entity> draggedEntities;
		std::vector<EntityFolder*> draggedFolders;

		for (const auto& pSelectedItem : selectedItems)
		{
			if (pSelectedItem->IsEntity())
				draggedEntities.push_back(pSelectedItem->AsEntity());
			else if (pSelectedItem->IsFolder())
				draggedFolders.push_back(pSelectedItem->AsFolder());
		}

		pEntityDragOp->SetDraggedEntities(std::move(draggedEntities));
		pEntityDragOp->SetDraggedFolders(std::move(draggedFolders));
		
		const Ref<OutlinerListItem>& pPrimaryDraggedItem = m_pOutlinerTreeView->GetItemFromWidget(pRow);

		if (pPrimaryDraggedItem->IsEntity())
		{
			const String tooltipText = std::format(ICON_FA_BAN "   {}. Cannot attach entity to self.", GetRowName(pRow));
			pEntityDragOp->SetTooltipText(tooltipText);
		}
		else if (pPrimaryDraggedItem->IsFolder())
		{
			const String tooltipText = std::format(ICON_FA_BAN "   {}. Cannot attach folder to self.", GetRowName(pRow));
			pEntityDragOp->SetTooltipText(tooltipText);
		}

		return pEntityDragOp;
	}

	bool EntityOutlinerView::OnDragEnter(OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return false;

		if (dragDropOp.GetDraggedEntities().empty() && dragDropOp.GetDraggedFolders().empty())
			return false;

		const Ref<OutlinerListItem>& pHoveredItem = m_pOutlinerTreeView->GetItemFromWidget(pRow);
		Scene* pScene = pEditor->GetActiveScene();

		EntityOutlinerPolicies::MoveContext context
		{
			.Scene = *pScene,
			.FoldersManager = *pEditor->GetEntityFoldersManager(),
			.TargetPayload = pHoveredItem->Payload,
			.Entities = dragDropOp.GetDraggedEntities(),
			.Folders = dragDropOp.GetDraggedFolders()
		};

		const EntityOutlinerPolicies::ValidationResponse response = m_pPolicies->ValidateMoveRequest(context);
		const String tooltipText = (response.IsValid ? ICON_FA_CHECK"   " : ICON_FA_BAN"   ") + response.Message;
		dragDropOp.SetTooltipText(tooltipText);
		
		return response.IsValid;
	}

	bool EntityOutlinerView::OnDrop(OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return false;

		const Ref<OutlinerListItem>& pDropTargetItem = m_pOutlinerTreeView->GetItemFromWidget(pRow);
		const std::vector<entity>& draggedEntities = dragDropOp.GetDraggedEntities();
		Scene& scene = *pEditor->GetActiveScene();
		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();

		EntityOutlinerPolicies::MoveContext context
		{
			.Scene = scene,
			.FoldersManager = *pEditor->GetEntityFoldersManager(),
			.TargetPayload = pDropTargetItem->Payload,
			.Entities = dragDropOp.GetDraggedEntities(),
			.Folders = dragDropOp.GetDraggedFolders()
		};

		const EntityOutlinerPolicies::MovePlan movePlan = m_pPolicies->ResolveMoveRequest(context);
		ExecuteMovePlan(movePlan, scene, pDropTargetItem->Payload);

		return true;
	}

	std::vector<EntityFolder*> EntityOutlinerView::MergeFoldersByLabel(const std::vector<EntityFolder*>& someFolders) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return someFolders;

		if (someFolders.empty() || someFolders.size() == 1)
			return someFolders;

		Scene& scene = *pEditor->GetActiveScene();

		std::unordered_map<String, std::vector<EntityFolder*>> labelToFoldersMap;

		for (EntityFolder* pFolder : someFolders)
			labelToFoldersMap[pFolder->GetLabel()].push_back(pFolder);

		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();
		std::unordered_set<EntityFolder*> foldersToDelete;

		for (auto& [Label, folders] : labelToFoldersMap)
		{
			if (folders.size() == 1)
				continue;

			//Merge by moving all entities and folders in all but the first folder to the first folder.
			EntityFolder* pTargetFolder = folders.front();
			const Folder targetRawFolder = Folder(FolderRoot::CreateFromScene(scene), pTargetFolder->GetPath());
			for (uint32 i = 1u; i < folders.size(); ++i)
			{
				EntityFolder* pCurrrentFolder = folders[i];

				std::unordered_set<String> folderPaths;
				folderPaths.insert(pCurrrentFolder->GetPath());

				EntityFoldersManager::ForEachEntityInFolders(scene, folderPaths, [&](entity aEntity)
					{
						pFoldersManager->AttachEntityToFolder(scene, aEntity, targetRawFolder);
						return true;
					});

				foldersToDelete.insert(pCurrrentFolder);
			}
		}

		std::vector<EntityFolder*> foldersToReturn;
		for (EntityFolder* pFolder : someFolders)
		{
			if (!foldersToDelete.contains(pFolder))
				foldersToReturn.push_back(pFolder);
		}

		for (EntityFolder* pFolder : foldersToDelete)
			pEditor->GetEntityFoldersManager()->DeleteFolder(scene, pFolder->GetPath());

		return foldersToReturn;
	}

	void EntityOutlinerView::OnEntityAttached(entity child, entity parent) noexcept
	{
		Ref<OutlinerListItem> pSceneItem = GetRootSceneItem();
		Ref<OutlinerListItem> pChildItem = nullptr;

		if (auto it = std::find_if(pSceneItem->Children.begin(), pSceneItem->Children.end(), [&](const Ref<OutlinerListItem>& pListItem) { return pListItem->IsEntity() && pListItem->AsEntity() == child; }); it != pSceneItem->Children.end())
		{
			pChildItem = *it;
			std::iter_swap(it, pSceneItem->Children.end() - 1);
			pSceneItem->Children.pop_back();
		}

		GetEntityItem(parent)->Children.push_back(pChildItem);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityCreated(entity newEntity) noexcept
	{
		GetRootSceneItem()->Children.push_back(CreateEntityListItem(newEntity));
		m_pOutlinerTreeView->RequestTreeRefresh();

		UpdateEntityInfoBorder();
	}

	void EntityOutlinerView::OnEntityDestroyed(entity aDestroyedEntity) noexcept
	{
		UpdateEntityInfoBorder();
	}

	void EntityOutlinerView::OnEntityPreDestroyed(entity destroyedEntity) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		const Ref<OutlinerListItem>& pDestroyedEntityItem = GetEntityItem(destroyedEntity);

		if (m_pOutlinerTreeView->IsItemSelected(pDestroyedEntityItem))
			m_pOutlinerTreeView->SetItemSelection(pDestroyedEntityItem, ESelectionType::Deselected);

		if (m_pOutlinerTreeView->IsItemHighlighted(pDestroyedEntityItem))
			m_pOutlinerTreeView->SetItemHighlighted(pDestroyedEntityItem, false);

		if (EntityFolder* pFolder = pEditor->GetFolderContainingEntity(destroyedEntity))
		{
			if (m_FolderToItemMap.contains(pFolder->GetUUID()))
				std::erase_if(m_FolderToItemMap.at(pFolder->GetUUID())->Children, [destroyedEntity](const Ref<OutlinerListItem>& pItem) { return pItem->IsEntity() && pItem->AsEntity() == destroyedEntity; });
		}
		else
			std::erase_if(GetRootSceneItem()->Children, [destroyedEntity](const Ref<OutlinerListItem>& pItem) { return pItem->IsEntity() && pItem->AsEntity() == destroyedEntity; });
		
		m_EntityToItemMap.erase(pEditor->GetActiveScene()->GetEntityManager().Get<IDComponent>(destroyedEntity).UuId);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityDetached(entity child, entity parent) noexcept
	{
		const Ref<OutlinerListItem>& pParentItem = GetEntityItem(parent);

		auto it = std::find_if(pParentItem->Children.begin(), pParentItem->Children.end(), [child](const Ref<OutlinerListItem>& pChild) { return pChild->AsEntity() == child; });
		if (it != pParentItem->Children.end())
		{
			GetRootSceneItem()->Children.push_back(*it);
			pParentItem->Children.erase(it);
		}

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityRemovedFromFolder(entity aEntity, const Folder& aFolder) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		Scene& scene = *pEditor->GetActiveScene();

		Ref<EntityFolder> pEntityFolder = pEditor->GetEntityFoldersManager()->GetFolder(scene, aFolder.GetPath());
		const Ref<OutlinerListItem>& pParentItem = GetFolderItem(pEntityFolder);

		auto it = std::find_if(pParentItem->Children.begin(), pParentItem->Children.end(), [aEntity](const Ref<OutlinerListItem>& pChild) { return pChild->IsEntity() && pChild->AsEntity() == aEntity; });
		if (it != pParentItem->Children.end())
		{
			GetRootSceneItem()->Children.push_back(*it);
			pParentItem->Children.erase(it);
		}

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityAttachedToFolder(entity aEntity, const Folder& aFolder) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		//If it has been a child of an entity or folder it will at this point already have been detached. As such only checking root is required.

		Scene& scene = *pEditor->GetActiveScene();

		Ref<EntityFolder> pFolder = pEditor->GetEntityFoldersManager()->GetFolder(scene, aFolder.GetPath());

		std::erase_if(GetRootSceneItem()->Children, [aEntity](const Ref<OutlinerListItem>& pItem) { return pItem->IsEntity() && pItem->AsEntity() == aEntity; });

		const Ref<OutlinerListItem>& pFolderItem = GetFolderItem(pFolder);
		pFolderItem->Children.push_back(m_EntityToItemMap.at(scene.GetEntityManager().Get<IDComponent>(aEntity).UuId));

		if (m_pOutlinerTreeView->ExistsItemInfo(pFolderItem))
			m_pOutlinerTreeView->SetItemExpandedState(pFolderItem, true);
		
		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityVisibilityChanged(entity aEntity, bool aVisibilityState) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		Scene& scene = *pEditor->GetActiveScene();
		EntityManager& entityManager = scene.GetEntityManager();

		if (!m_EntityToItemMap.contains(entityManager.Get<IDComponent>(aEntity).UuId))
			return;

		const uint32 numEntities = entityManager.GetEntityAliveCount();
		const uint32 numHiddenEntities = entityManager.Collect<HiddenInGameComponent>().Size();
		OutlinerTableRow* pSceneOutlinerTableRow = static_cast<OutlinerTableRow*>(m_pOutlinerTreeView->GetRowWidget(GetRootSceneItem()).Get());
		Button* pSceneVisibilityButton = pSceneOutlinerTableRow->GetVisibilityButton();
		if (numHiddenEntities == numEntities)
		{
			pSceneVisibilityButton->SetText(ICON_FA_EYE_SLASH);
			pSceneVisibilityButton->SetIsVisible(m_SceneItemSelected || !aVisibilityState || pSceneVisibilityButton->IsHovered());
		}
		else
		{
			pSceneVisibilityButton->SetText(ICON_FA_EYE);
			pSceneVisibilityButton->SetIsVisible(m_SceneItemSelected || pSceneVisibilityButton->IsHovered());
		}

		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();
		
		if (EntityFolder* pFolder = pEditor->GetFolderContainingEntity(aEntity))
		{
			while (pFolder)
			{
				//We need to determine for current folder, and ALL ancestor folders whether their visibility should change.

				const Ref<OutlinerListItem>& pFolderListItem = GetFolderItem(pFolder);
				if (!m_pOutlinerTreeView->IsItemVisible(pFolderListItem))
				{
					pFolder = pFolder->GetParent();
					continue;
				}

				std::unordered_set<String> folderPaths;
				pFoldersManager->ForEachFolderWithRootObject(FolderRoot::CreateFromScene(scene), [&](const EntityFolder& aFolder)
					{
						if (aFolder.GetPath() == pFolder->GetPath() || FilePath::IsSubpath(aFolder.GetPath(), pFolder->GetPath()))
							folderPaths.insert(aFolder.GetPath());

						return true;
					});

				std::vector<entity> folderEntities;
				EntityFoldersManager::ForEachEntityInFolders(scene, folderPaths, [&](entity aCurrentEntity)
					{
						folderEntities.push_back(aCurrentEntity);
						return true;
					});

				const bool shouldChangeFolderVisibility = !folderEntities.empty() && std::all_of(folderEntities.begin(), folderEntities.end(), [&](entity aCurrentEntity) { return scene.IsEntityVisible(aCurrentEntity) == aVisibilityState; });

				if (shouldChangeFolderVisibility)
				{
					OutlinerTableRow* pFolderTableRow = static_cast<OutlinerTableRow*>(m_pOutlinerTreeView->GetRowWidget(GetFolderItem(pFolder)).Get());
					Button* pFolderVisibilityButton = pFolderTableRow->GetVisibilityButton();
					pFolderVisibilityButton->SetText(aVisibilityState ? ICON_FA_EYE : ICON_FA_EYE_SLASH);

					pFolderVisibilityButton->SetIsVisible(m_SelectedFolders.contains(pFolder->GetUUID()) || !aVisibilityState || pFolderVisibilityButton->IsHovered());
				}

				pFolder = pFolder->GetParent();
			}
		}

		const Ref<OutlinerListItem>& pListItem = GetEntityItem(aEntity);
		if (!m_pOutlinerTreeView->IsItemVisible(pListItem))
			return;

		const bool isSelected = m_pOutlinerTreeView->IsItemSelected(pListItem);

		Ref<ITableRow> pRow = m_pOutlinerTreeView->GetRowWidget(pListItem);
		OutlinerTableRow* pOutlinerTableRow = static_cast<OutlinerTableRow*>(pRow.Get());

		Button* pVisibilityButton = pOutlinerTableRow->GetVisibilityButton();

		pVisibilityButton->SetText(aVisibilityState ? ICON_FA_EYE : ICON_FA_EYE_SLASH);
		pVisibilityButton->SetIsVisible(isSelected || !aVisibilityState || pVisibilityButton->IsHovered());
	}

	void EntityOutlinerView::OnExpandCollapseButtonClicked(Button* pButton, Ref<OutlinerListItem> pItem) noexcept
	{
		const bool isExpanded = m_pOutlinerTreeView->GetItemInfo(pItem).IsExpanded;
		m_pOutlinerTreeView->SetItemExpandedState(pItem, !isExpanded);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnFolderCreated(EntityFolder* pFolder) noexcept
	{
		if (EntityFolder* pParent = pFolder->GetParent())
			GetFolderItem(pParent)->Children.push_back(CreateEntityFolderListItem(pFolder));
		else
			GetRootSceneItem()->Children.push_back(CreateEntityFolderListItem(pFolder));
		
		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnFolderMoved(EntityFolder* pMovedFolder, EntityFolder* paOldParentFolder, const String& aOldPath, const String& aNewPath) noexcept
	{
		if (paOldParentFolder && m_FolderToItemMap.contains(paOldParentFolder->GetUUID()))
			std::erase_if(GetFolderItem(paOldParentFolder)->Children, [pMovedFolder](const Ref<OutlinerListItem>& pItem) { return pItem->IsFolder() && pItem->AsFolder() == pMovedFolder; });
		else
			std::erase_if(GetRootSceneItem()->Children, [pMovedFolder](const Ref<OutlinerListItem>& pItem) { return pItem->IsFolder() && pItem->AsFolder() == pMovedFolder; });

		if (EntityFolder* pNewParent = pMovedFolder->GetParent())
		{
			GetFolderItem(pNewParent)->Children.push_back(GetFolderItem(pMovedFolder));

			if (m_pOutlinerTreeView->IsItemVisible(GetFolderItem(pNewParent)))
				m_pOutlinerTreeView->SetItemExpandedState(GetFolderItem(pNewParent), true);
		}
		else
			GetRootSceneItem()->Children.push_back(GetFolderItem(pMovedFolder));

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityFolderDeleted(EntityFolder* pFolder) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		const Ref<OutlinerListItem>& pFolderItem = GetFolderItem(pFolder);
		Scene& scene = *pEditor->GetActiveScene();

		if (m_pOutlinerTreeView->IsItemSelected(pFolderItem))
			m_pOutlinerTreeView->SetItemSelection(pFolderItem, ESelectionType::Deselected);

		if (m_pOutlinerTreeView->IsItemHighlighted(pFolderItem))
			m_pOutlinerTreeView->SetItemHighlighted(pFolderItem, false);

		//Strategy: Deleting a folder should incur a move of all entities and folders within to first valid ancestor (if none -> scene root item)
		EntityFolder* pAncestorFolder = pFolder->GetParent();
		while (pAncestorFolder && pAncestorFolder->IsMarkedAsDeleted())
			pAncestorFolder = pAncestorFolder->GetParent();

		std::vector<Ref<OutlinerListItem>> children = pFolderItem->Children;
		if (pAncestorFolder)
		{
			const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();

			for (auto& pChildItem : children)
			{
				if (pChildItem->IsEntity())
					pFoldersManager->AttachEntityToFolder(scene, pChildItem->AsEntity(), Folder(FolderRoot::CreateFromScene(*pEditor->GetActiveScene()), pAncestorFolder->GetPath()));
			}
		}
		else
		{
			GetRootSceneItem()->Children.insert(GetRootSceneItem()->Children.end(), children.begin(), children.end());
		}

		if (EntityFolder* pParentFolder = pFolder->GetParent())
			std::erase_if(GetFolderItem(pParentFolder)->Children, [pFolder](const Ref<OutlinerListItem>& pItem) { return pItem->IsFolder() && pItem->AsFolder() == pFolder; });
		else
			std::erase_if(GetRootSceneItem()->Children, [pFolder](const Ref<OutlinerListItem>& pItem) { return pItem->IsFolder() && pItem->AsFolder() == pFolder; });
		
		m_FolderToItemMap.erase(pFolder->GetUUID());
		m_SelectedFolders.erase(pFolder->GetUUID());

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnPreSceneChanged(Scene* pScene) noexcept
	{
		if (!pScene)
			return;

		pScene->OnEntityCreated.Detach(this);
		pScene->OnEntityPreDestroyed.Detach(this);
		pScene->OnEntityDestroyed.Detach(this);
		pScene->OnEntityAttached.Detach(this);
		pScene->OnEntityDetached.Detach(this);
		pScene->OnEntityVisibilityChanged.Detach(this);
	}

	void EntityOutlinerView::OnFocusChanged(bool focused) noexcept
	{
		m_pOutlinerTreeView->TriggerFocusChange(focused);
	}

	Ref<ITableRow> EntityOutlinerView::OnGenerateRow(const Ref<OutlinerListItem>& item) noexcept
	{
		RLS_SCOPED_SUSPEND();

		auto pEditor = Editor::Get();
		if (!pEditor)
			return nullptr;

		Scene& scene = *pEditor->GetActiveScene();
		const ItemInfo& info = m_pOutlinerTreeView->GetItemInfo(item);

		OutlinerTableRowCreateInfo createInfo;
		createInfo.pTreeView = m_pOutlinerTreeView;
		createInfo.HasChildren = info.HasChildren;
		createInfo.IsExpanded = info.IsExpanded;

		if (item->IsEntity())
		{
			createInfo.Icon = ICON_FA_CUBE;
			createInfo.Name = scene.GetEntityManager().Get<NameComponent>(item->AsEntity()).Name;
			createInfo.Type = "Entity";
			createInfo.IsSelected = pEditor->GetSelection()->IsEntitySelected(item->AsEntity());
			createInfo.IsVisible = scene.IsEntityVisible(item->AsEntity());
		}
		else if (item->IsFolder())
		{
			createInfo.Icon = (info.HasChildren && info.IsExpanded) ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER;
			createInfo.Name = item->AsFolder()->GetLabel();
			createInfo.Type = "Folder";
			createInfo.IsSelected = m_SelectedFolders.contains(item->AsFolder()->GetUUID());
			constexpr Color folderIconColor = Colors::Normalize(213.0f, 166.0f, 74.0f, 255.0f);
			createInfo.IconColor = folderIconColor;

			const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();

			std::unordered_set<String> folderPaths;

			const String rootPath = item->AsFolder()->GetPath();

			pFoldersManager->ForEachFolderWithRootObject(FolderRoot::CreateFromScene(scene), [&](const EntityFolder& aFolder)
				{
					if (aFolder.GetPath() == rootPath || FilePath::IsSubpath(aFolder.GetPath(), rootPath))
						folderPaths.insert(aFolder.GetPath());

					return true;
				});

			std::vector<entity> folderEntities;

			EntityFoldersManager::ForEachEntityInFolders(scene, folderPaths, [&scene, &folderEntities](entity aEntity)
				{
					folderEntities.push_back(aEntity);
					return true;
				});

			createInfo.IsVisible = !folderEntities.empty() && std::all_of(folderEntities.begin(), folderEntities.end(), [&](entity aEntity) { return scene.IsEntityVisible(aEntity); });
		}
		else
		{
			RLS_ASSERT(item->IsScene(), "[EntityOutlinerView::OnGenerateRow]: Unknown item type encountered.");

			createInfo.Icon = ICON_FA_SITEMAP;
			createInfo.Name = item->AsScene()->GetName();
			createInfo.Type = "Scene";
			createInfo.IsSelected = m_SceneItemSelected;
			
			EntityManager& entityManager = item->AsScene()->GetEntityManager();
			createInfo.IsVisible = entityManager.Collect<HiddenInGameComponent>().Size() != entityManager.GetEntityAliveCount();
		}

		if (createInfo.IsSelected && !m_pOutlinerTreeView->IsItemSelected(item))
			m_pOutlinerTreeView->SetItemSelection(item, ESelectionType::Selected);
		else if (!createInfo.IsSelected && m_pOutlinerTreeView->IsItemSelected(item))
			m_pOutlinerTreeView->SetItemSelection(item, ESelectionType::Deselected);

		Ref<OutlinerTableRow> pRow = new OutlinerTableRow(createInfo);
		pRow->OnMouseEnter(this, &EntityOutlinerView::OnMouseEnterRow);
		pRow->OnMouseExit(this, &EntityOutlinerView::OnMouseExitRow);
		pRow->OnDragDetected(this, &EntityOutlinerView::OnDragDetected);
		pRow->OnDragEnter(this, &EntityOutlinerView::OnDragEnter);
		pRow->OnDrop(this, &EntityOutlinerView::OnDrop);

		Button* pVisibilityButton = pRow->GetVisibilityButton();
		pVisibilityButton
			->OnClicked(std::bind(&EntityOutlinerView::OnVisibilityButtonClicked, this, pVisibilityButton, item))
			->OnMouseEnter(this, &EntityOutlinerView::OnMouseEnterVisibilityButton)
			->OnMouseExit(std::bind(&EntityOutlinerView::OnMouseExitVisibilityButton, this, pVisibilityButton, item));

		Button* pExpandButton = pRow->GetExpandButton();
		pExpandButton
			->OnClicked(std::bind(&EntityOutlinerView::OnExpandCollapseButtonClicked, this, pExpandButton, item))
			->OnMouseEnter(this, &EntityOutlinerView::OnMouseEnterExpandCollapseButton)
			->OnMouseExit(this, &EntityOutlinerView::OnMouseExitExpandCollapseButton);
		
		const bool highlighted = m_pOutlinerTreeView->IsItemHighlighted(item);

		if (highlighted && m_pFilter->TestTextFilter(createInfo.Name, ETextFilterTextComparisonMode::Partial))
			pRow->GetNameLabel()->SetHighlightedSubstring(m_pFilter->GetFilterText());

		if (highlighted && m_pFilter->TestTextFilter(createInfo.Type, ETextFilterTextComparisonMode::Partial))
			pRow->GetTypeLabel()->SetHighlightedSubstring(m_pFilter->GetFilterText());

		return pRow;
	}

	void EntityOutlinerView::OnGetChildren(const Ref<OutlinerListItem>& pParent, std::vector<Ref<OutlinerListItem>>& outChildren) noexcept
	{
		outChildren = pParent->Children;
	}

	void EntityOutlinerView::OnItemScrolledIntoView(const Ref<OutlinerListItem>& aItem) noexcept
	{
		if (m_pFolderToRenameWhenScrolledIntoView == aItem)
		{
			if (m_pOutlinerTreeView->GetNumItemsSelected() != 1 && m_pOutlinerTreeView->IsItemSelected(m_pFolderToRenameWhenScrolledIntoView))
			{
				m_pOutlinerTreeView->ClearSelection();
				m_pOutlinerTreeView->SetItemSelection(m_pFolderToRenameWhenScrolledIntoView, ESelectionType::Selected);
			}

			OnRenameSelection();
		}
	}

	void EntityOutlinerView::OnMouseEnterVisibilityButton(Button* pButton) noexcept
	{
		pButton->SetAlpha(1.0f);
	}

	void EntityOutlinerView::OnMouseExitVisibilityButton(Button* pButton, OutlinerListItem* pItem) noexcept
	{
		if (!m_pOutlinerTreeView->IsItemSelected(pItem))
			pButton->SetAlpha(0.7f);
	}

	void EntityOutlinerView::OnMouseEnterExpandCollapseButton(Button* pButton) noexcept
	{
		if (!pButton->IsEnabled())
			return;

		pButton->SetAlpha(1.0f);
	}

	void EntityOutlinerView::OnMouseExitExpandCollapseButton(Button* pButton) noexcept
	{
		if (!pButton->IsEnabled())
			return;

		pButton->SetAlpha(0.7f);
	}

	void EntityOutlinerView::OnMouseEnterRow(ITableRow* pTableRow) noexcept
	{
		static_cast<OutlinerTableRow*>(pTableRow)->GetVisibilityButton()->SetIsVisible(true);
	}

	void EntityOutlinerView::OnMouseExitRow(ITableRow* pTableRow) noexcept
	{
		if (m_pOutlinerTreeView->IsItemSelected(m_pOutlinerTreeView->GetItemFromWidget(pTableRow)))
			return;

		Button* pButton = static_cast<OutlinerTableRow*>(pTableRow)->GetVisibilityButton();

		if (pButton->GetText() == ICON_FA_EYE)
			pButton->SetIsVisible(false);
	}

	void EntityOutlinerView::OnOutlinerTreeRefreshed() noexcept
	{
		if (m_pItemToScrollIntoView)
		{
			m_pOutlinerTreeView->RequestScrollToItem(m_pItemToScrollIntoView);
			m_pItemToScrollIntoView = nullptr;
		}
	}

	void EntityOutlinerView::OnRender() noexcept
	{
		PROFILE_FUNC;
		m_pMainBox->Render();
	}

	void EntityOutlinerView::OnRenameSelection() noexcept
	{
		std::vector<Ref<OutlinerListItem>> selectedItems;
		if (m_pOutlinerTreeView->GetSelectedItems(selectedItems) != 1)
			return;

		if (selectedItems.front()->IsScene())
			return;

		if (!m_pOutlinerTreeView->IsItemVisible(selectedItems.front()))
			return;

		Ref<ITableRow> pRow = m_pOutlinerTreeView->GetRowWidget(selectedItems.front());
		OutlinerTableRow* pOutlinerRow = static_cast<OutlinerTableRow*>(pRow.Get());
		WidgetSwitcher* pWidgetSwitcher = pOutlinerRow->GetWidgetSwitcher();
		EditableTextBox* pEditableTextBox = pOutlinerRow->GetEditableTextBox();
		pEditableTextBox->SetText(pOutlinerRow->GetNameLabel()->GetText());
		pEditableTextBox->ForceKeyboardFocus();

		pEditableTextBox->OnTextChanged([pListItem = selectedItems.front().Get(), pEditableTextBox, this](const char* pText)
			{
				auto pEditor = Editor::Get();
				if (!pEditor)
					return;

				const String& currentName = GetItemName(pListItem);
				const String inputName = pText;

				Color targetColor = Colors::White;
				if (pListItem->IsFolder())
				{
					EntityFolder* pFolder = pListItem->AsFolder();
					EntityFolder* pParentFolder = pFolder->GetParent();
					const String suggestedLabel = pEditor->GetEntityFoldersManager()->GetFolderName(*pEditor->GetActiveScene(), pParentFolder ? pParentFolder->GetPath() : "", inputName);
					targetColor = suggestedLabel != inputName && currentName != inputName ? Colors::Red : Colors::White;
				}
				
				Application::Get().SubmitToMainThread([targetColor, pEditableTextBox](){ pEditableTextBox->SetTextColor(targetColor); });
			});

		pEditableTextBox->OnTextCommitted([pListItem = selectedItems.front().Get(), pWidgetSwitcher, pEditableTextBox, this](const char* aText, ETextCommitType aCommitType)
			{
				const String name = aText;
				Application::Get().SubmitToMainThread([&, name, commitType = aCommitType]()
					{
						const String& currentLabel = GetItemName(pListItem);
						const String newName = name;

						if (currentLabel == newName)
						{
							pWidgetSwitcher->SetActiveWidgetIndex(0);
							return;
						}
						else if (newName.empty() && commitType == ETextCommitType::OnUserMovedFocus)
						{
							pWidgetSwitcher->SetActiveWidgetIndex(0);
							return;
						}
						else if (newName.empty() && commitType == ETextCommitType::OnEnter)
						{
							pEditableTextBox->SetText(currentLabel);
							pEditableTextBox->ForceKeyboardFocus();
							return;
						}

						if (!newName.empty() && commitType == ETextCommitType::OnUserMovedFocus)
						{
							if (pListItem->IsFolder() && RenameFolder(pListItem->AsFolder(), newName))
							{
								//TODO: Hightlight request
							}
							else if (pListItem->IsEntity() && RenameEntity(pListItem->AsEntity(), newName))
							{
								//TODO: Hightlight request
							}

							pWidgetSwitcher->SetActiveWidgetIndex(0);
						}
						else if (!newName.empty() && commitType == ETextCommitType::OnEnter)
						{
							if (pListItem->IsFolder() && RenameFolder(pListItem->AsFolder(), newName))
							{
								//TODO: Hightlight request
							}
							else if (pListItem->IsEntity() && RenameEntity(pListItem->AsEntity(), newName))
							{
								//TODO: Hightlight request
							}
							else
							{
								pEditableTextBox->SetText(currentLabel);
								pEditableTextBox->ForceKeyboardFocus();
								return;
							}

							pWidgetSwitcher->SetActiveWidgetIndex(0);
						}

						RecreateItemHierarchy();
						m_pOutlinerTreeView->RequestTreeRefresh();
					});
			});

		pWidgetSwitcher->SetActiveWidgetIndex(1);
	}

	const std::vector<Ref<OutlinerListItem>>* EntityOutlinerView::OnRequestSource() noexcept
	{
		//TODO: A Sorting mechanism a bit more advanced, supporting ascending and descending strings.

		auto&& RecursiveSort = [&](auto&& Self, std::vector<Ref<OutlinerListItem>>& items) -> void
			{
				auto&& GetItemRank = [](const Ref<OutlinerListItem>& pItem) noexcept -> int 
					{
						if (pItem->IsEntity())
							return 2;
						else if (pItem->IsFolder())
							return 1;
						else
							return 0;
					};

				std::sort(items.begin(), items.end(), [&](const Ref<OutlinerListItem>& pItemA, const Ref<OutlinerListItem>& pItemB)
					{
						const int rankA = GetItemRank(pItemA);
						const int rankB = GetItemRank(pItemB);
						if (rankA != rankB)
							return rankA < rankB;

						return StringUtils::ToLower(GetItemName(pItemA)) < StringUtils::ToLower(GetItemName(pItemB));
					});

				for (const auto& item : items)
					Self(Self, item->Children);
			};

		RecursiveSort(RecursiveSort, m_ListItems);

		return &m_ListItems;
	}

	void EntityOutlinerView::OnRowDoubleClicked(const Ref<OutlinerListItem>& apItem) noexcept
	{
		if (apItem->IsFolder())
			m_pOutlinerTreeView->SetItemExpandedState(apItem, !m_pOutlinerTreeView->GetItemInfo(apItem).IsExpanded);
	}

	void EntityOutlinerView::OnSceneChanged(Scene* pScene) noexcept
	{
		m_EntityToItemMap.clear();
		m_FolderToItemMap.clear();
		m_ListItems.clear();
		m_SelectedFolders.clear();
		m_SceneItemSelected = false;

		m_pOutlinerTreeView->RequestTreeRefresh();

		if (!pScene)
			return;

		pScene->OnEntityCreated.Connect(this, &EntityOutlinerView::OnEntityCreated);
		pScene->OnEntityPreDestroyed.Connect(this, &EntityOutlinerView::OnEntityPreDestroyed);
		pScene->OnEntityDestroyed.Connect(this, &EntityOutlinerView::OnEntityDestroyed);
		pScene->OnEntityAttached.Connect(this, &EntityOutlinerView::OnEntityAttached);
		pScene->OnEntityDetached.Connect(this, &EntityOutlinerView::OnEntityDetached);
		pScene->OnEntityVisibilityChanged.Connect(this, &EntityOutlinerView::OnEntityVisibilityChanged);

		RecreateItemHierarchy();
		UpdateEntityInfoBorder();
	}

	void EntityOutlinerView::OnSearchTextChanged(const char* pText) noexcept
	{
		RLS_SCOPED_SUSPEND();

		m_pOutlinerTreeView->ClearSelection();
		m_pOutlinerTreeView->ClearHightlightedItems();
		m_ListItems.clear();
		m_EntityToItemMap.clear();
		m_FolderToItemMap.clear();
		
		m_pFilter->SetFilterText(pText);

		RecreateItemHierarchy();
		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnSearchTextCommitted(const char* pText, ETextCommitType commitType) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		if (commitType != ETextCommitType::OnEnter)
			return;

		if (std::strlen(pText) == 0u)
			return;

		m_pOutlinerTreeView->ClearSelection();

		const bool containsEntityLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetEntityTypeAsString(), ETextFilterTextComparisonMode::Partial);

		EntityManager& mgr = pEditor->GetActiveScene()->GetEntityManager();

		auto&& RecursiveSelectEligibleItems = [&](auto&& Self, const std::vector<Ref<OutlinerListItem>>& items) -> void
			{
				for (size_t i = 0u; i < items.size(); ++i)
				{
					const bool isEntityItem = items[i]->IsEntity();

					if (!m_pOutlinerTreeView->IsItemSelected(items[i]) && m_pFilter->TestTextFilter(isEntityItem ? mgr.Get<NameComponent>(items[i]->AsEntity()).Name : "?", ETextFilterTextComparisonMode::Partial))
						m_pOutlinerTreeView->SetItemSelection(items[i], ESelectionType::Selected);

					Self(Self, items[i]->Children);
				}
			};

		RecursiveSelectEligibleItems(RecursiveSelectEligibleItems, m_ListItems);
	}

	void EntityOutlinerView::OnSelectionChanged(const Ref<OutlinerListItem>& item, ESelectionType selectionType) noexcept
	{
		if (m_SuspendNotifications)
			return;

		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		RLS_SCOPED_SUSPEND();

		const UniquePtr<Selection>& pSelection = pEditor->GetSelection();

		OutlinerTableRow* pOutlinerTableRow = nullptr;
		Button* pVisibilityButton = nullptr;

		if (m_pOutlinerTreeView->IsItemVisible(item))
		{
			pOutlinerTableRow = static_cast<OutlinerTableRow*>(m_pOutlinerTreeView->GetRowWidget(item).Get());
			pVisibilityButton = pOutlinerTableRow->GetVisibilityButton();
		}

		if (selectionType == ESelectionType::Selected)
		{
			if (item->IsEntity())
				pSelection->SelectEntity(item->AsEntity());
			else if (item->IsFolder())
				m_SelectedFolders.insert(item->AsFolder()->GetUUID());
			else if (item->IsScene())
				m_SceneItemSelected = true;

			if (pVisibilityButton)
			{
				pVisibilityButton->SetIsVisible(true);
				pVisibilityButton->SetAlpha(1.0f);
			}
		}
		else
		{
			if (item->IsEntity())
				pSelection->DeselectEntity(item->AsEntity());
			else if (item->IsFolder())
				m_SelectedFolders.erase(item->AsFolder()->GetUUID());
			else if (item->IsScene())
				m_SceneItemSelected = false;

			if (pOutlinerTableRow && !pOutlinerTableRow->IsHovered())
			{	
				if (pVisibilityButton->GetText() == ICON_FA_EYE)
					pVisibilityButton->SetIsVisible(false);
				
				pVisibilityButton->SetAlpha(0.7f);
			}
		}
	}

	void EntityOutlinerView::OnSelectionChangedExternally(entity aEntity, ESelectionState aSelectionState) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		UpdateEntityInfoBorder();

		if (m_SuspendNotifications)
			return;

		Scene& scene = *pEditor->GetActiveScene();

		{
			RLS_SCOPED_SUSPEND();
			m_pOutlinerTreeView->SetItemSelection(m_EntityToItemMap.at(scene.GetEntityManager().Get<IDComponent>(aEntity).UuId), (ESelectionType)aSelectionState);
		}

		if (!m_pOutlinerTreeView->IsItemVisible(m_EntityToItemMap.at(scene.GetEntityManager().Get<IDComponent>(aEntity).UuId)))
			return;

		OutlinerTableRow* pOutlinerTableRow = static_cast<OutlinerTableRow*>(m_pOutlinerTreeView->GetRowWidget(m_EntityToItemMap[scene.GetEntityManager().Get<IDComponent>(aEntity).UuId]).Get());

		switch (aSelectionState)
		{
		case ESelectionState::Selected:
		{
			pOutlinerTableRow->GetVisibilityButton()->SetIsVisible(true);
			pOutlinerTableRow->GetVisibilityButton()->SetAlpha(1.0f);
			break;
		}
		case ESelectionState::Deselected:
		{
			if (!pOutlinerTableRow->IsHovered() && (pOutlinerTableRow->GetVisibilityButton()->GetText() != ICON_FA_EYE_SLASH))
			{
				pOutlinerTableRow->GetVisibilityButton()->SetIsVisible(false);
				pOutlinerTableRow->GetVisibilityButton()->SetAlpha(0.7f);
			}

			DeselectAllFolders();

			if (m_SceneItemSelected)
			{
				OnSelectionChanged(GetRootSceneItem(), ESelectionType::Deselected);
				m_pOutlinerTreeView->SetItemSelection(GetRootSceneItem(), ESelectionType::Deselected);
			}

			break;
		}
		}
	}

	void EntityOutlinerView::OnVisibilityButtonClicked(Button* pButton, Ref<OutlinerListItem> pItem) noexcept
	{
		const bool isVisible = (pButton->GetText() == ICON_FA_EYE);
		const bool newVisibility = !isVisible;

		ToggleVisibilityForItem(pItem, newVisibility);

		if (m_pOutlinerTreeView->IsItemSelected(pItem))
		{
			std::vector<Ref<OutlinerListItem>> otherSelectedItems;
			m_pOutlinerTreeView->GetSelectedItems(otherSelectedItems);

			std::erase_if(otherSelectedItems, [pItem](const Ref<OutlinerListItem>& pCurrentItem) { return pCurrentItem == pItem; });
		
			for (const auto& pOtherItem : otherSelectedItems)
				ToggleVisibilityForItem(pOtherItem, newVisibility);
		}
	}

	void EntityOutlinerView::RecreateItemHierarchy() noexcept
	{
		/*
			STRATEGY:
			1. Add Scene as root item.
			2. For every root folder:
			2.1 Add it, then recursively add its children, along with their respective entities and entities children.
			2.2 This takes care of all folders and entities that are in folders , as well as entities that are children of entities in folders.
			3. For every root entity that is not in a folder, add it, then recursively add its children.

			Problem: Any entry should be added if it or any of its descendants match the text filter, else it shouldn't.
			Solution: When adding an entry, check if it or any of its descendants match the text filter, if so add it, else skip it.
		*/

		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		m_pOutlinerTreeView->ClearSelection();
		m_pOutlinerTreeView->ClearHightlightedItems();
		m_ListItems.clear();
		m_EntityToItemMap.clear();
		m_FolderToItemMap.clear();

		Scene& scene = *pEditor->GetActiveScene();
		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();
		const UniquePtr<Selection>& pSelection = pEditor->GetSelection();

		std::vector<entity> entitiesToAdd;
		std::vector<String> foldersToAdd;

		const bool containsEntityLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetEntityTypeAsString(), ETextFilterTextComparisonMode::Partial);
		const bool containsFolderLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetFolderTypeAsString(), ETextFilterTextComparisonMode::Partial);
		const bool containsSceneLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetSceneTypeAsString(), ETextFilterTextComparisonMode::Partial);

		pFoldersManager->ForEachFolderWithRootObject(FolderRoot::CreateFromScene(scene), [&](const EntityFolder& aEntityFolder)
			{
				if (m_pFilter->TestTextFilter(aEntityFolder.GetLabel(), ETextFilterTextComparisonMode::Partial) || containsFolderLabel)
				{
					foldersToAdd.push_back(aEntityFolder.GetPath());
					return true;
				}

				//We need ALL descendant folders and ALL entities belonging to those folders AND all entities descending from THOSE entities as well!

				std::unordered_set<String> descendantFolderLabels;
				std::unordered_set<String> descendantFolderPaths;

				//Descendant folders:
				pFoldersManager->ForEachFolderWithRootObject(FolderRoot::CreateFromScene(scene), [&](const EntityFolder& aCandidateDescendantFolder)
					{
						if (aEntityFolder.GetUUID() == aCandidateDescendantFolder.GetUUID())
							return true;

						if (aCandidateDescendantFolder.GetPath().starts_with(aEntityFolder.GetPath()))
						{
							descendantFolderLabels.insert(aCandidateDescendantFolder.GetLabel());
							descendantFolderPaths.insert(aCandidateDescendantFolder.GetPath());
						}

						return true;
					});

				if (std::ranges::any_of(descendantFolderLabels, [&](const String& aDescendantLabel)
					{
						return m_pFilter->TestTextFilter(aDescendantLabel, ETextFilterTextComparisonMode::Partial);
					}))
				{
					foldersToAdd.push_back(aEntityFolder.GetPath());
					return true;
				}

				descendantFolderPaths.insert(aEntityFolder.GetPath());

				//All entities for all those folders:
				EntityFoldersManager::ForEachEntityInFolders(scene, descendantFolderPaths, [&](entity aCurrentEntity)
					{
						if (m_pFilter->TestTextFilter(scene.GetEntityManager().Get<NameComponent>(aCurrentEntity).Name, ETextFilterTextComparisonMode::Partial) || containsEntityLabel)
						{
							foldersToAdd.push_back(aEntityFolder.GetPath());
							return false;
						}

						//Test descendants:
						scene.ForEachEntityWithAncestorEntity(aCurrentEntity, true, [&](entity currentDescendant)
							{
								if (m_pFilter->TestTextFilter(scene.GetEntityManager().Get<NameComponent>(currentDescendant).Name, ETextFilterTextComparisonMode::Partial) || containsEntityLabel)
								{
									foldersToAdd.push_back(aEntityFolder.GetPath());
									return false;
								}
								return true;
							});

						return true;
					});

				return true;
			});

		auto&& ConditionallyAddEntity = [&](entity currentEntity)
			{
				scene.ForEachEntityWithAncestorEntity(currentEntity, true, [&](entity e)
					{
						if (m_pFilter->TestTextFilter(scene.GetEntityManager().Get<NameComponent>(e).Name, ETextFilterTextComparisonMode::Partial) || containsEntityLabel)
						{
							entitiesToAdd.push_back(currentEntity);
							return false;
						}
						return true;
					});
			};

		scene.GetEntityManager().Collect<IDComponent, RootComponent>().Do([&](entity e)
			{
				scene.ForEachEntityWithAncestorEntity(e, true, [&](entity currentEntity)
					{
						ConditionallyAddEntity(currentEntity);
						return true;
					});
			});


		enum class NodeCat : uint8_t { Root = 0, Internal = 1, Leaf = 2 };

		auto category = [&](entity e) -> NodeCat 
			{
				const bool p = scene.HasParent(e);
				if (!p)
					return NodeCat::Root;

				return scene.IsParent(e) ? NodeCat::Internal : NodeCat::Leaf;
			};

		// Optional: add tie-breakers after category (Id, Name, Priority…)
		auto key = [&](entity e) 
			{
				return std::tuple{ static_cast<uint8_t>(category(e)), /* e.Priority, e.Id, ... */ };
			};

		std::stable_sort(entitiesToAdd.begin(), entitiesToAdd.end(), [&](entity a, entity b) 
			{ 
				return key(a) < key(b); 
			});

		std::stable_sort(foldersToAdd.begin(), foldersToAdd.end(), [&](const String& a, const String& b)
			{
				return StringUtils::Split(a, '/').size() < StringUtils::Split(b, '/').size();
			});

		//We now know what folders and entities to add, so we can proceed to add them.
		if (!foldersToAdd.empty() || !entitiesToAdd.empty())
		{
			Ref<OutlinerListItem> pSceneListItem = CreateSceneListItem(pEditor->GetActiveScene());
			if (m_SceneItemSelected)
				m_pOutlinerTreeView->SetItemSelection(pSceneListItem, ESelectionType::Selected);

			if (m_pFilter->TestTextFilter(scene.GetName(), ETextFilterTextComparisonMode::Partial) || containsSceneLabel)
				m_pOutlinerTreeView->SetItemHighlighted(pSceneListItem, true);

			m_ListItems.push_back(pSceneListItem);

			for (const String& folderPath : foldersToAdd)
			{
				EntityFolder* pFolder = pFoldersManager->GetFolder(scene, folderPath);

				Ref<OutlinerListItem> pFolderListItem = CreateEntityFolderListItem(pFolder);
				if (m_SelectedFolders.contains(pFolder->GetUUID()))
					m_pOutlinerTreeView->SetItemSelection(pFolderListItem, ESelectionType::Selected);

				if (m_pFilter->TestTextFilter(pFolder->GetLabel(), ETextFilterTextComparisonMode::Partial) || containsFolderLabel)
					m_pOutlinerTreeView->SetItemHighlighted(pFolderListItem, true);

				if (pFolder->GetParent() && m_FolderToItemMap.contains(pFolder->GetParent()->GetUUID()))
					m_FolderToItemMap[pFolder->GetParent()->GetUUID()]->Children.push_back(pFolderListItem);
				else
					GetRootSceneItem()->Children.push_back(pFolderListItem);
			}
			for (entity e : entitiesToAdd)
			{
				Ref<OutlinerListItem> pEntityListItem = CreateEntityListItem(e);
				if (pSelection->IsEntitySelected(e))
					m_pOutlinerTreeView->SetItemSelection(pEntityListItem, ESelectionType::Selected);

				if (m_pFilter->TestTextFilter(scene.GetEntityManager().Get<NameComponent>(e).Name, ETextFilterTextComparisonMode::Partial) || containsEntityLabel)
					m_pOutlinerTreeView->SetItemHighlighted(pEntityListItem, true);

				if (scene.EntityHasAncestors(e))
					m_EntityToItemMap[scene.GetEntityManager().Get<IDComponent>(scene.GetParent(e)).UuId]->Children.push_back(pEntityListItem);
				else if (EntityFolder* pFolder = pEditor->GetFolderContainingEntity(e))
					m_FolderToItemMap[pFolder->GetUUID()]->Children.push_back(pEntityListItem);
				else
					GetRootSceneItem()->Children.push_back(pEntityListItem);
			}
		}
		else
		{
			if (m_pFilter->TestTextFilter(scene.GetName(), ETextFilterTextComparisonMode::Partial) || containsSceneLabel)
			{
				Ref<OutlinerListItem> pSceneListItem = CreateSceneListItem(pEditor->GetActiveScene());
				if (m_SceneItemSelected)
					m_pOutlinerTreeView->SetItemSelection(pSceneListItem, ESelectionType::Selected);

				m_pOutlinerTreeView->SetItemHighlighted(pSceneListItem, true);
				m_ListItems.push_back(pSceneListItem);
			}
		}
	}

	bool EntityOutlinerView::RenameFolder(EntityFolder* aFolder, const String& aName) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return false;

		EntityFolder* pParentFolder = aFolder->GetParent();
		const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();
		Scene& scene = *pEditor->GetActiveScene();

		const String suggestedLabel = pFoldersManager->GetFolderName(scene, pParentFolder ? pParentFolder->GetPath() : "", aName);
		if (suggestedLabel != aName || aFolder->GetLabel() == aName)
			return false;

		return pFoldersManager->RenameFolder(scene, aFolder->GetPath(), pParentFolder ? FilepathUtils::CombineDisplay(pParentFolder->GetPath(), aName) : aName);
	}

	bool EntityOutlinerView::RenameEntity(entity aEntity, const String& aName) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return false;

		NameComponent& nameComponent = pEditor->GetActiveScene()->GetEntityManager().Get<NameComponent>(aEntity);
		if (nameComponent.Name == aName)
			return false;

		nameComponent.Name = aName;
		return true;
	}

	void EntityOutlinerView::ToggleVisibilityForItem(const Ref<OutlinerListItem>& pAOutlinerListItem, bool aToVisible) noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		Scene& scene = *pEditor->GetActiveScene();

		if (pAOutlinerListItem->IsEntity())
			scene.SetEntityVisibleInGame(pAOutlinerListItem->AsEntity(), aToVisible);
		else if (pAOutlinerListItem->IsFolder())
		{
			const UniquePtr<EntityFoldersManager>& pFoldersManager = pEditor->GetEntityFoldersManager();

			const String rootPath = pAOutlinerListItem->AsFolder()->GetPath();

			std::unordered_set<String> folderPaths;

			pFoldersManager->ForEachFolderWithRootObject(FolderRoot::CreateFromScene(scene), [&](const EntityFolder& aFolder)
				{
					if (aFolder.GetPath().starts_with(rootPath))
						folderPaths.insert(aFolder.GetPath());

					return true;
				});

			EntityFoldersManager::ForEachEntityInFolders(scene, folderPaths, [&scene, aToVisible](entity aEntity)
				{
					scene.SetEntityVisibleInGame(aEntity, aToVisible);
					return true;
				});
		}
		else
			scene.GetEntityManager().Collect<IDComponent>().Do([&scene, aToVisible](entity aEntity) { scene.SetEntityVisibleInGame(aEntity, aToVisible); });
	}

	void EntityOutlinerView::UpdateEntityInfoBorder() noexcept
	{
		auto pEditor = Editor::Get();
		if (!pEditor)
			return;

		const UniquePtr<Selection>& pSelection = pEditor->GetSelection();
		if (!pSelection)
			return;

		Scene* pScene = pEditor->GetActiveScene();
		if (!pScene)
			return;

		VerticalBoxEx* pContentWidget = m_pMainBox->GetWidget<VerticalBoxEx>(2);
		if (!pContentWidget)
			return;

		Border* pInfoBorder = pContentWidget->GetWidget<Border>(0);

		Label* pLabel = static_cast<Label*>(static_cast<HorizontalBoxEx*>(pInfoBorder->GetContent().Get())->GetWidget<Label>(0));
		if (!pLabel)
			return;

		const uint32 numTotalEntities = pScene->GetEntityManager().GetEntityAliveCount();
		const uint32 numSelectedEntities = pSelection->GetSelectedEntityCount();
		const String entityLabel = numTotalEntities == 1u ? "entity" : "entities";

		pLabel->SetText(std::format("{} {}{}", numTotalEntities, entityLabel, numSelectedEntities > 0u ? std::format(" ({} selected)", numSelectedEntities) : ""));
	}
}
