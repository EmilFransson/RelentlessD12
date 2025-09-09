#include "EntityOutlinerView.h"

#include "../../../Core/Editor.h"

namespace Relentless
{
	EntityOutlinerView::EntityOutlinerView(Editor* pEditor) noexcept
		: m_pEditor{pEditor}
	{
		m_pEditor->OnPreSceneChanged.Connect(this, &EntityOutlinerView::OnPreSceneChanged);
		m_pEditor->OnSceneChanged.Connect(this, &EntityOutlinerView::OnSceneChanged);

		m_pEditor->GetSelection()->OnSelectionChanged.Connect(this, &EntityOutlinerView::OnSelectionChangedExternally);
		
		const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();
		
		pFilterManager->OnFilterCreated.Connect(this, &EntityOutlinerView::OnFilterCreated);
		pFilterManager->OnFilterDestroyed.Connect(this, &EntityOutlinerView::OnFilterDestroyed);
		pFilterManager->OnEntitySetToFilter.Connect(this, &EntityOutlinerView::OnEntitySetToFilter);
		pFilterManager->OnEntityRemovedFromFilter.Connect(this, &EntityOutlinerView::OnEntityRemovedFromFilter);
		pFilterManager->OnFilterReattached.Connect(this, &EntityOutlinerView::OnFilterReattached);

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
			->OnDebugItemToString(this, &EntityOutlinerView::OnDebugItemToString)
			->OnSelectionChanged(this, &EntityOutlinerView::OnSelectionChanged)
			->OnContextMenuOpening(this, &EntityOutlinerView::OnContextMenuOpening);

		m_pMainBox = new VerticalBox();
		
		Ref<HorizontalBox> pHorizontalBox = new HorizontalBox();
		pHorizontalBox->SetMargin(FloatRect(0.0f, 10.0f, 0.0f, 0.0f));

		pHorizontalBox->Add(new Button(ICON_FA_FOLDER_PLUS))
			->OnClicked(this, &EntityOutlinerView::OnCreateNewFilterButtonClicked)
			->SetFont(ImGui::GetIO().Fonts->Fonts[2])
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Gray)
			->SetBorderColor(Colors::Transparent)
			->SetTooltipText("Create a new filter containing the current selection");

		pHorizontalBox->Add(new SearchBar("Search...", true))
			->OnTextChanged(this, &EntityOutlinerView::OnSearchTextChanged)
			->OnTextCommitted(this, &EntityOutlinerView::OnSearchTextCommitted)
			->SetSizePolicy(ESizePolicy::Stretch);

		m_pOutlinerListBox = new VerticalBox(Vector2::Zero, true);
		m_pOutlinerListBox->OnFocusChanged.Connect(this, &EntityOutlinerView::OnFocusChanged);
		m_pOutlinerListBox->Add(m_pOutlinerTreeView);

		m_pMainBox->Add(pHorizontalBox);
		m_pMainBox->Add(m_pOutlinerListBox);

		m_pFilter = std::make_unique<TextFilterExpressionEvaluator>();

		if (Scene* pScene = m_pEditor->GetActiveScene())
			OnSceneChanged(pScene);
	}

	EntityOutlinerView::~EntityOutlinerView() noexcept
	{
		if (const UniquePtr<Selection>& pSelection = m_pEditor->GetSelection())
			pSelection->OnSelectionChanged.Detach(this);

		if (const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager())
		{
			pFilterManager->OnFilterCreated.Detach(this);
			pFilterManager->OnFilterDestroyed.Detach(this);
			pFilterManager->OnEntitySetToFilter.Detach(this);
			pFilterManager->OnEntityRemovedFromFilter.Detach(this);
			pFilterManager->OnFilterReattached.Detach(this);
		}

		if (Scene* pScene = m_pEditor->GetActiveScene())
		{
			pScene->OnEntityCreated.Detach(this);
			pScene->OnEntityDestroyed.Detach(this);
			pScene->OnEntityAttached.Detach(this);
			pScene->OnEntityDetached.Detach(this);
			pScene->OnEntityVisibilityChanged.Detach(this);
		}
	}

	Ref<OutlinerListItem> EntityOutlinerView::CreateEntityListItem(entity e) noexcept
	{
		Ref<OutlinerListItem> pEntityItem = new OutlinerListItem();
		pEntityItem->Entity = e;

		m_EntityToItemMap[e] = pEntityItem;

		return pEntityItem;
	}

	Ref<OutlinerListItem> EntityOutlinerView::CreateEntityFilterListItem(EntityFilter* pFilter) noexcept
	{
		Ref<OutlinerListItem> pFilterListItem = new OutlinerListItem();
		pFilterListItem->pFilter = pFilter;
		
		m_FilterToItemMap[pFilter] = pFilterListItem;

		return pFilterListItem;
	}

	Ref<OutlinerListItem> EntityOutlinerView::CreateSceneListItem(Scene* pScene) noexcept
	{
		Ref<OutlinerListItem> pSceneListItem = new OutlinerListItem();
		pSceneListItem->pScene = pScene;

		return pSceneListItem;
	}

	const String& EntityOutlinerView::GetItemName(const Ref<OutlinerListItem>& pItem) const noexcept
	{
		if (pItem->IsEntityItem())
			return m_pEditor->GetActiveScene()->GetEntityManager().Get<NameComponent>(pItem->Entity).Name;
		else if (pItem->IsFilterItem())
			return pItem->pFilter->GetName();
		else
			return pItem->pScene->GetName();
	}

	const String& EntityOutlinerView::GetRowName(const OutlinerTableRow* pRow) const noexcept
	{
		return GetItemName(m_pOutlinerTreeView->GetItemFromWidget(pRow));
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

	void EntityOutlinerView::OnCreateNewFilterButtonClicked() noexcept
	{
		const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();
		Scene* pScene = m_pEditor->GetActiveScene();

		std::vector<Ref<OutlinerListItem>> selectedItems;
		m_pOutlinerTreeView->GetSelectedItems(selectedItems);

		//Step 1: Create the new filter. If 1 existing filter is selected -> assign it as parent for the new filter. 
		// Otherwise, the new filter should itself become a root filter.

		String newFilterPath = "";

		if (selectedItems.size() == 1 && selectedItems.front()->IsFilterItem())
		{
			EntityFilter* pParentFilter = selectedItems.front()->pFilter;

			String newFilterName = "";
			uint32 filterIndex = 0;

			bool nameOccupied = true;

			while (nameOccupied)
			{
				nameOccupied = false;
				filterIndex++;

				newFilterName = std::format("NewFilter{}", filterIndex);

				pFilterManager->ForEachFilterWithParentObject(pParentFilter, [&](EntityFilter* pChildFilter)
					{
						nameOccupied = pChildFilter->GetName() == newFilterName;
						
						return !nameOccupied;
					});
			}

			newFilterPath = pParentFilter->GetPath() + "/" + newFilterName;
			selectedItems.clear();
		}
		else
		{
			//Should have no parent -> should be created as a root filter:
			String newFilterName = "";
			uint32 filterIndex = 0;

			bool nameOccupied = true;

			while (nameOccupied)
			{
				nameOccupied = false;
				filterIndex++;

				newFilterName = std::format("NewFilter{}", filterIndex);

				pFilterManager->ForEachRootFilters([&](EntityFilter* pRootFilter)
					{
						nameOccupied = pRootFilter->GetName() == newFilterName;

						return !nameOccupied;
					});
			}

			newFilterPath = newFilterName;
		}

		EntityFilter* pNewFilter = pFilterManager->CreateFilter(newFilterPath);

		//Step 2: Assign all selected filters and entities to the newly created filter.
		
		std::vector<entity> selectedEntities;
		std::vector<EntityFilter*> selectedFilters;

		for (const auto& pSelectedItem : selectedItems)
		{
			if (pSelectedItem->IsEntityItem())
				selectedEntities.push_back(pSelectedItem->Entity);
			else if (pSelectedItem->IsFilterItem())
				selectedFilters.push_back(pSelectedItem->pFilter);
			//Ignore scene item
		}

		//Step 2.1: Remove all entities from the selected entities that are descendants to other selected entities.
		//Attach remaining entities to newly created filter.
		std::erase_if(selectedEntities, [&](entity candidateDescendant)
			{  
				return std::any_of(selectedEntities.begin(), selectedEntities.end(), [&](entity candidateAncestor)
					{
						return pScene->EntityIsDescendant(candidateAncestor, candidateDescendant);
					});
			});

		for (entity selectedEntity : selectedEntities)
		{
			if (pScene->EntityHasAncestors(selectedEntity))
				pScene->DetachEntity(selectedEntity);

			pFilterManager->SetEntityToFilter(selectedEntity, pNewFilter->GetPath());
		}

		//Step 2.2: Remaining selected filters should next be attached to the new filter.
		//Name clashes are first resolved, followed by attaching the filters.

		for (EntityFilter* pSelectedFilter : selectedFilters)
		{
			bool nameClash = std::any_of(selectedFilters.begin(), selectedFilters.end(), [pSelectedFilter](EntityFilter* pCurrentFilter)
				{
					return pSelectedFilter != pCurrentFilter && pSelectedFilter->GetName() == pCurrentFilter->GetName();
				});

			if (nameClash)
			{
				String newFilterName = "";
				uint32 filterIndex = 0;

				if (auto num = StringUtils::ExtractTrailingNumber(pSelectedFilter->GetName()))
				{
					filterIndex = static_cast<uint32>(*num);
					newFilterName = StringUtils::StripTrailingDigits(pSelectedFilter->GetName());
				}
				else
					newFilterName = pSelectedFilter->GetName();

				while (nameClash)
				{
					newFilterName = std::format("{}{}", newFilterName, ++filterIndex);
					nameClash = std::any_of(selectedFilters.begin(), selectedFilters.end(), [&newFilterName](EntityFilter* pCurrentFilter) 
						{  
							return pCurrentFilter->GetName() == newFilterName;
						});
				}

				pSelectedFilter->SetName(newFilterName);
			}
		}

		for (EntityFilter* pSelectedFilter : selectedFilters)
			pFilterManager->SetFilterToFilter(pSelectedFilter->GetPath(), pNewFilter->GetPath());
	}

	void EntityOutlinerView::OnDeleteSelection() noexcept
	{
		std::vector<Ref<OutlinerListItem>> selectedItems;
		if (m_pOutlinerTreeView->GetSelectedItems(selectedItems) == 0)
			return;

		for (const Ref<OutlinerListItem>& pItem : selectedItems)
		{
			if (pItem->IsEntityItem())
				m_pEditor->GetActiveScene()->DestroyEntity(pItem->Entity);
			else if (pItem->IsFilterItem())
				m_pEditor->GetEntityFiltersManager()->DestroyFilter(pItem->pFilter->GetPath());
		}
	}

	void EntityOutlinerView::OnDuplicateSelection() noexcept
	{
		//TODO: Consider deferring to Editor

		std::vector<Ref<OutlinerListItem>> selectedItems;
		if (m_pOutlinerTreeView->GetSelectedItems(selectedItems) == 0)
			return;

		Scene* pScene = m_pEditor->GetActiveScene();
		const UniquePtr<Selection>& pSelection = m_pEditor->GetSelection();

		pSelection->DeselectAllEntities();

		std::unordered_set<entity> roots;

		for (size_t i = 0u; i < selectedItems.size(); ++i)
		{
			RLS_ASSERT(selectedItems[i]->IsEntityItem(), "UNIMPLEMENTED!");

			if (!std::any_of(selectedItems.begin(), selectedItems.end(), [&](const Ref<OutlinerListItem>& pItem)
				{
					return pItem->IsEntityItem() && pScene->EntityIsParent(selectedItems[i]->Entity, pItem->Entity);
				}))
			{
				roots.insert(selectedItems[i]->Entity);
				//TODO: Erase root list items.
			}
		}

		auto&& RecurseDuplicate = [&](auto&& Self, entity toDuplicate, entity parent = NULL_ENTITY) -> void
			{
				const entity duplicatedEntity = m_pEditor->GetActiveScene()->DuplicateEntity(toDuplicate, false);
				pSelection->SelectEntity(duplicatedEntity);

				if (parent != NULL_ENTITY)
					pScene->AttachEntity(duplicatedEntity, parent);

				for (const Ref<OutlinerListItem>& pItem : selectedItems)
				{
					if (pScene->EntityIsChild(pItem->Entity, toDuplicate))
						Self(Self, pItem->Entity, duplicatedEntity);
				}
			};

		for (entity e : roots)
			RecurseDuplicate(RecurseDuplicate, e);
	}

	String EntityOutlinerView::OnDebugItemToString(const Ref<OutlinerListItem>& item) const noexcept
	{
		if (item->IsEntityItem())
			return std::format("Entity: {}", item->Entity);
		else if (item->IsFilterItem())
			return std::format("Filter: {}", item->pFilter->GetName());
		else 
			return std::format("Scene: {}", item->pScene->GetName());
	}

	Ref<DragDropOperation> EntityOutlinerView::OnDragDetected(OutlinerTableRow* pRow) noexcept
	{
		Ref<OutlinerDragDropOperation> pEntityDragOp = new OutlinerDragDropOperation(pRow);

		std::vector<Ref<OutlinerListItem>> selectedItems;
		m_pOutlinerTreeView->GetSelectedItems(selectedItems);

		std::vector<entity> draggedEntities;
		std::vector<EntityFilter*> draggedFilters;

		for (const auto& pSelectedItem : selectedItems)
		{
			if (pSelectedItem->IsEntityItem())
				draggedEntities.push_back(pSelectedItem->Entity);
			else if (pSelectedItem->IsFilterItem())
				draggedFilters.push_back(pSelectedItem->pFilter);
		}

		pEntityDragOp->SetDraggedEntities(std::move(draggedEntities));
		pEntityDragOp->SetDraggedFilters(std::move(draggedFilters));
		
		const Ref<OutlinerListItem>& pPrimaryDraggedItem = m_pOutlinerTreeView->GetItemFromWidget(pRow);

		if (pPrimaryDraggedItem->IsEntityItem())
		{
			const String tooltipText = std::format(ICON_FA_BAN "   {}. Cannot attach entity to self.", GetRowName(pRow));
			pEntityDragOp->SetTooltipText(tooltipText);
		}
		else if (pPrimaryDraggedItem->IsFilterItem())
		{
			const String tooltipText = std::format(ICON_FA_BAN "   {}. Cannot attach filter to self.", GetRowName(pRow));
			pEntityDragOp->SetTooltipText(tooltipText);
		}

		return pEntityDragOp;
	}

	bool EntityOutlinerView::OnDragEnter(OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) noexcept
	{
		if (dragDropOp.GetDraggedEntities().empty() && dragDropOp.GetDraggedFilters().empty())
			return false;

		const Ref<OutlinerListItem>& pHoveredItem = m_pOutlinerTreeView->GetItemFromWidget(pRow);

		if (m_pOutlinerTreeView->IsItemSelected(pHoveredItem) && !pHoveredItem->IsSceneItem())
		{
			const String tooltipText = std::format(ICON_FA_BAN "   {}. Cannot attach entity to self.", GetItemName(pHoveredItem));
			dragDropOp.SetTooltipText(tooltipText);
			return false;
		}
		else
		{
			Scene* pScene = m_pEditor->GetActiveScene();

			if (pHoveredItem->IsEntityItem())
			{
				if (std::any_of(dragDropOp.GetDraggedEntities().begin(), dragDropOp.GetDraggedEntities().end(), [&](entity e)
					{
						return pScene->EntityIsDescendant(e, pHoveredItem->Entity);
					}))
				{
					const String tooltipText = std::format(ICON_FA_BAN "   {}. Parent cannot become the child of their descendant.", GetItemName(pHoveredItem));
					dragDropOp.SetTooltipText(tooltipText);
					return false;
				}
				else if (!dragDropOp.GetDraggedFilters().empty())
				{
					const String tooltipText = std::format(ICON_FA_BAN "   {}.", GetItemName(pHoveredItem));
					dragDropOp.SetTooltipText(tooltipText);
					return false;
				}
			}
			else if (pHoveredItem->IsFilterItem())
			{
				entity containedEntity = NULL_ENTITY;
				EntityFilter* pContainedFilter = nullptr;
				const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();

				if (std::any_of(dragDropOp.GetDraggedEntities().begin(), dragDropOp.GetDraggedEntities().end(), [&](entity e)
					{
						containedEntity = e;
						return pHoveredItem->pFilter->Contains(e);
					}))
				{
					const String tooltipText = std::format(ICON_FA_BAN "   {} is already assigned to {}.", pScene->GetEntityManager().Get<NameComponent>(containedEntity).Name, pHoveredItem->pFilter->GetPath());
					dragDropOp.SetTooltipText(tooltipText);
					return false;
				}
				else if (std::any_of(dragDropOp.GetDraggedFilters().begin(), dragDropOp.GetDraggedFilters().end(), [&](EntityFilter* pFilter)
					{
						bool anyIsDescendant = false;
						pFilterManager->ForEachFilterWithRootObject(pFilter, false, [&](EntityFilter* pDescendant)
							{
								anyIsDescendant = pDescendant == pHoveredItem->pFilter;
								if (anyIsDescendant)
									return false;
								else
									return true;
							});

						return anyIsDescendant;
					}))
				{
					const String tooltipText = std::format(ICON_FA_BAN "   {}. Parent cannot become the child of their descendant.", GetItemName(pHoveredItem));
					dragDropOp.SetTooltipText(tooltipText);
					return false;
				}
				else if (std::any_of(dragDropOp.GetDraggedFilters().begin(), dragDropOp.GetDraggedFilters().end(), [&](EntityFilter* pFilter)
					{
						pContainedFilter = pFilter;
						return pHoveredItem->pFilter->Contains(pFilter);
					}))
				{
					const String tooltipText = std::format(ICON_FA_BAN "   {} is already assigned to {}.", pContainedFilter->GetName(), pHoveredItem->pFilter->GetPath());
					dragDropOp.SetTooltipText(tooltipText);
					return false;
				}
				else if (std::any_of(dragDropOp.GetDraggedFilters().begin(), dragDropOp.GetDraggedFilters().end(), [&](EntityFilter* pFilter)
					{
						pContainedFilter = pFilter;
						return pHoveredItem->pFilter->FindChild(pFilter->GetName()) != nullptr;
					}))
				{
					const String tooltipText = std::format(ICON_FA_BAN "   {} already contains a filter child named '{}'.", pHoveredItem->pFilter->GetName(), pContainedFilter->GetName());
					dragDropOp.SetTooltipText(tooltipText);
					return false;
				}
			}
			else
			{
				RLS_ASSERT(pHoveredItem->IsSceneItem(), "[EntityOutlinerView::OnDragEnter]: Unknown item type encountered.");

				Scene* pScene = m_pEditor->GetActiveScene();
				const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();

				if (!dragDropOp.GetDraggedFilters().empty())
				{
					const String tooltipText = std::format(ICON_FA_BAN "   {}.", GetItemName(pHoveredItem));
					dragDropOp.SetTooltipText(tooltipText);
					return false;
				}
				else
				{
					entity containedEntity = NULL_ENTITY;
					if (std::any_of(dragDropOp.GetDraggedEntities().begin(), dragDropOp.GetDraggedEntities().end(), [&](entity e)
						{
							containedEntity = e;
							return !pScene->EntityHasAncestors(e) && !pFilterManager->IsEntityInAnyFilter(e);
						}))
					{
						const String tooltipText = std::format(ICON_FA_BAN "   {} is already assigned to root.", pScene->GetEntityManager().Get<NameComponent>(containedEntity).Name);
						dragDropOp.SetTooltipText(tooltipText);
						return false;
					}
				}
			}
		}
		
		if (pHoveredItem->IsEntityItem())
		{
			const String tooltipText = std::format(ICON_FA_CHECK "   {}.", GetItemName(pHoveredItem));
			dragDropOp.SetTooltipText(tooltipText);
		}
		else if (pHoveredItem->IsFilterItem())
		{
			const String tooltipText = std::format(ICON_FA_CHECK "   Move into '{}'.", pHoveredItem->pFilter->GetPath());
			dragDropOp.SetTooltipText(tooltipText);
		}
		else
		{
			const String tooltipText = std::format(ICON_FA_CHECK "   Move to root.");
			dragDropOp.SetTooltipText(tooltipText);
		}

		return true;
	}

	bool EntityOutlinerView::OnDrop(OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) noexcept
	{
		const Ref<OutlinerListItem>& pDropTargetItem = m_pOutlinerTreeView->GetItemFromWidget(pRow);

		const std::vector<entity>& draggedEntities = dragDropOp.GetDraggedEntities();

		if (pDropTargetItem->IsEntityItem())
		{
			Scene* pScene = m_pEditor->GetActiveScene();
			const bool shouldDetach = std::all_of(draggedEntities.begin(), draggedEntities.end(), [&](entity e) { return pScene->EntityIsChild(e, pDropTargetItem->Entity); });

			if (shouldDetach)
			{
				for (entity e : draggedEntities)
					pScene->DetachEntity(e);
			}
			else
			{
				for (entity e : draggedEntities)
					pScene->AttachEntity(e, pDropTargetItem->Entity);
			}
		}
		else if (pDropTargetItem->IsFilterItem())
		{
			const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();
			const std::vector<EntityFilter*>& draggedFilters = dragDropOp.GetDraggedFilters();

			for (entity e : draggedEntities)
				pFilterManager->SetEntityToFilter(e, pDropTargetItem->pFilter->GetPath());
			for (const auto& pFilter : draggedFilters)
				pFilterManager->SetFilterToFilter(pFilter->GetPath(), pDropTargetItem->pFilter->GetPath());
		}
		else
		{
			RLS_ASSERT(pDropTargetItem->IsSceneItem(), "[EntityOutlinerView::OnDrop]: Unknown item type encountered.");

			Scene* pScene = m_pEditor->GetActiveScene();
			const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();
			
			for (entity e : draggedEntities)
			{
				if (pScene->EntityHasAncestors(e))
					pScene->DetachEntity(e);

				if (pFilterManager->IsEntityInAnyFilter(e))
					pFilterManager->RemoveEntityFromCurrentFilter(e);
			}
		}

		return true;
	}

	void EntityOutlinerView::OnEntityAttached(entity child, entity parent) noexcept
	{
		Ref<OutlinerListItem> pChildItem = nullptr;

		if (auto it = std::find_if(m_ListItems[0]->Children.begin(), m_ListItems[0]->Children.end(), [&](const Ref<OutlinerListItem>& pListItem) { return pListItem->Entity == child; }); it != m_ListItems[0]->Children.end())
		{
			pChildItem = *it;
			std::iter_swap(it, m_ListItems[0]->Children.end() - 1);
			m_ListItems[0]->Children.pop_back();
		}

		m_EntityToItemMap[parent]->Children.push_back(pChildItem);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityCreated(entity newEntity) noexcept
	{
		m_ListItems[0]->Children.push_back(CreateEntityListItem(newEntity));
		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityDestroyed(entity destroyedEntity) noexcept
	{
		if (m_pOutlinerTreeView->IsItemSelected(m_EntityToItemMap[destroyedEntity]))
			m_pOutlinerTreeView->SetItemSelection(m_EntityToItemMap[destroyedEntity], ESelectionType::Deselected);

		if (m_pOutlinerTreeView->IsItemHighlighted(m_EntityToItemMap[destroyedEntity]))
			m_pOutlinerTreeView->SetItemHighlighted(m_EntityToItemMap[destroyedEntity], false);

		std::erase_if(m_ListItems[0]->Children, [destroyedEntity](const Ref<OutlinerListItem>& pItem) { return pItem->Entity == destroyedEntity; });
		m_EntityToItemMap.erase(destroyedEntity);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityDetached(entity child, entity parent) noexcept
	{
		Ref<OutlinerListItem> pParentItem = m_EntityToItemMap[parent];

		auto it = std::find_if(pParentItem->Children.begin(), pParentItem->Children.end(), [child](const Ref<OutlinerListItem>& pChild) { return pChild->Entity == child; });
		if (it != pParentItem->Children.end())
		{
			m_ListItems[0]->Children.push_back(*it);
			pParentItem->Children.erase(it);
		}

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityRemovedFromFilter(entity entity, EntityFilter* pParentFilter, bool filterToBeDestroyed) noexcept
	{
		Ref<OutlinerListItem> pParentItem = m_FilterToItemMap[pParentFilter];
		auto it = std::find_if(pParentItem->Children.begin(), pParentItem->Children.end(), [entity](const Ref<OutlinerListItem>& pChild) { return pChild->Entity == entity; });
		if (it != pParentItem->Children.end())
		{
			m_ListItems[0]->Children.push_back(*it);
			pParentItem->Children.erase(it);
		}

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntitySetToFilter(entity entity, EntityFilter* pFilter) noexcept
	{
		//If it has been a child of an entity or filter it will at this point already have been detached. As such only checking root is required.

		std::erase_if(m_ListItems[0]->Children, [entity](const Ref<OutlinerListItem>& pItem) { return pItem->Entity == entity; });

		m_FilterToItemMap[pFilter]->Children.push_back(m_EntityToItemMap[entity]);
		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnEntityVisibilityChanged(entity e, bool visibilityState) noexcept
	{
		if (!m_EntityToItemMap.contains(e))
			return;

		Scene* pScene = m_pEditor->GetActiveScene();
		EntityManager& entityManager = pScene->GetEntityManager();

		const uint32 numEntities = entityManager.GetEntityAliveCount();
		const uint32 numHiddenEntities = entityManager.Collect<HiddenInGameComponent>().Size();
		OutlinerTableRow* pSceneOutlinerTableRow = static_cast<OutlinerTableRow*>(m_pOutlinerTreeView->GetRowWidget(m_ListItems[0]).Get());
		Button* pSceneVisibilityButton = pSceneOutlinerTableRow->GetVisibilityButton();
		if (numHiddenEntities == numEntities)
		{
			pSceneVisibilityButton->SetText(ICON_FA_EYE_SLASH);
			pSceneVisibilityButton->SetIsVisible(!visibilityState || pSceneVisibilityButton->IsHovered());
		}
		else
		{
			pSceneVisibilityButton->SetText(ICON_FA_EYE);
			pSceneVisibilityButton->SetIsVisible(pSceneVisibilityButton->IsHovered());
		}

		const UniquePtr<EntityFiltersManager>& pFiltersManager = m_pEditor->GetEntityFiltersManager();
		if (pFiltersManager->IsEntityInAnyFilter(e))
		{
			EntityFilter* pFilter = pFiltersManager->GetFilterContainingEntity(e);

			while (pFilter)
			{
				if (!m_FilterToItemMap.contains(pFilter))
					return;

				const Ref<OutlinerListItem>& pFilterListItem = m_FilterToItemMap[pFilter];
				if (!m_pOutlinerTreeView->IsItemVisible(pFilterListItem))
					return;

				bool changeFilterVisibility = true;

				pFiltersManager->ForEachFilterWithRootObject(pFilter, false, [&](EntityFilter* pCurrentFilter)
					{
						const std::unordered_set<entity>& filterEntities = pCurrentFilter->GetEntities();
						if (std::all_of(filterEntities.begin(), filterEntities.end(), [&](entity filterEntity)
							{
								return pScene->IsEntityVisible(filterEntity) == visibilityState;
							}))
						{
							return true;
						}
						else
						{
							changeFilterVisibility = false;
							return false;
						}
					});

				if (changeFilterVisibility)
				{
					OutlinerTableRow* pFilterTableRow = static_cast<OutlinerTableRow*>(m_pOutlinerTreeView->GetRowWidget(m_FilterToItemMap[pFilter]).Get());
					Button* pFilterVisibilityButton = pFilterTableRow->GetVisibilityButton();
					pFilterVisibilityButton->SetText(visibilityState ? ICON_FA_EYE : ICON_FA_EYE_SLASH);
					pFilterVisibilityButton->SetIsVisible(!visibilityState || pFilterVisibilityButton->IsHovered());
				}

				pFilter = pFilter->GetParent();
			}
		}

		const Ref<OutlinerListItem>& pListItem = m_EntityToItemMap[e];
		if (!m_pOutlinerTreeView->IsItemVisible(pListItem))
			return;

		Ref<ITableRow> pRow = m_pOutlinerTreeView->GetRowWidget(pListItem);
		OutlinerTableRow* pOutlinerTableRow = static_cast<OutlinerTableRow*>(pRow.Get());

		Button* pVisibilityButton = pOutlinerTableRow->GetVisibilityButton();

		pVisibilityButton->SetText(visibilityState ? ICON_FA_EYE : ICON_FA_EYE_SLASH);
		pVisibilityButton->SetIsVisible(!visibilityState || pVisibilityButton->IsHovered());
	}

	void EntityOutlinerView::OnExpandCollapseButtonClicked(Button* pButton, Ref<OutlinerListItem> pItem) noexcept
	{
		const bool isExpanded = m_pOutlinerTreeView->GetItemInfo(pItem).IsExpanded;
		m_pOutlinerTreeView->SetItemExpandedState(pItem, !isExpanded);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnFilterCreated(EntityFilter* pFilter) noexcept
	{
		if (EntityFilter* pParent = pFilter->GetParent())
			m_FilterToItemMap[pParent]->Children.push_back(CreateEntityFilterListItem(pFilter));
		else
			m_ListItems[0]->Children.push_back(CreateEntityFilterListItem(pFilter));
		
		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnFilterReattached(EntityFilter* pChild, EntityFilter* pPreviousParent, EntityFilter* pNewParent) noexcept
	{
		if (pPreviousParent)
			std::erase_if(m_FilterToItemMap[pPreviousParent]->Children, [pChild](const Ref<OutlinerListItem>& pItem) { return pItem->IsFilterItem() && pItem->pFilter == pChild; });
		else
			std::erase_if(m_ListItems[0]->Children, [pChild](const Ref<OutlinerListItem>& pItem) { return pItem->IsFilterItem() && pItem->pFilter == pChild; });

		if (pNewParent)
			m_FilterToItemMap[pNewParent]->Children.push_back(m_FilterToItemMap[pChild]);
		else
			m_ListItems[0]->Children.push_back(m_FilterToItemMap[pChild]);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnFilterDestroyed(EntityFilter* pFilter) noexcept
	{
		if (m_pOutlinerTreeView->IsItemSelected(m_FilterToItemMap[pFilter]))
			m_pOutlinerTreeView->SetItemSelection(m_FilterToItemMap[pFilter], ESelectionType::Deselected);

		if (m_pOutlinerTreeView->IsItemHighlighted(m_FilterToItemMap[pFilter]))
			m_pOutlinerTreeView->SetItemHighlighted(m_FilterToItemMap[pFilter], false);

		if (EntityFilter* pParentFilter = pFilter->GetParent())
			std::erase_if(m_FilterToItemMap[pParentFilter]->Children, [pFilter](const Ref<OutlinerListItem>& pItem) { return pItem->IsFilterItem() && pItem->pFilter == pFilter; });
		else
			std::erase_if(m_ListItems[0]->Children, [pFilter](const Ref<OutlinerListItem>& pItem) { return pItem->IsFilterItem() && pItem->pFilter == pFilter; });
		
		m_FilterToItemMap.erase(pFilter);
		m_SelectedFilters.erase(pFilter);

		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnPreSceneChanged(Scene* pScene) noexcept
	{
		if (!pScene)
			return;

		pScene->OnEntityCreated.Detach(this);
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
		m_SuspendNotifications = true;

		const ItemInfo& info = m_pOutlinerTreeView->GetItemInfo(item);

		OutlinerTableRowCreateInfo createInfo;
		createInfo.pTreeView = m_pOutlinerTreeView;
		createInfo.HasChildren = info.HasChildren;
		createInfo.IsExpanded = info.IsExpanded;

		if (item->IsEntityItem())
		{
			createInfo.Icon = ICON_FA_CUBE;
			createInfo.Name = m_pEditor->GetActiveScene()->GetEntityManager().Get<NameComponent>(item->Entity).Name;
			createInfo.Type = "Entity";
			createInfo.IsSelected = m_pEditor->GetSelection()->IsEntitySelected(item->Entity);
			createInfo.IsVisible = m_pEditor->GetActiveScene()->IsEntityVisible(item->Entity);
		}
		else if (item->IsFilterItem())
		{
			createInfo.Icon = (info.HasChildren && info.IsExpanded) ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER;
			createInfo.Name = item->pFilter->GetName();
			createInfo.Type = "Filter";
			createInfo.IsSelected = m_SelectedFilters.contains(item->pFilter);
			constexpr Color folderIconColor = Colors::Normalize(213.0f, 166.0f, 74.0f, 255.0f);
			createInfo.IconColor = folderIconColor;

			Scene* pScene = m_pEditor->GetActiveScene();
			const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();

			pFilterManager->ForEachFilterWithRootObject(item->pFilter, true, [&](EntityFilter* pFilter)
				{
					const std::unordered_set<entity> filterEntities = pFilter->GetEntities();
					if (filterEntities.empty())
						return true;

					createInfo.IsVisible = std::all_of(filterEntities.begin(), filterEntities.end(), [&](entity filterEntity)
						{
							return pScene->IsEntityVisible(filterEntity);
						});
				
					return createInfo.IsVisible;
				});
		}
		else
		{
			RLS_ASSERT(item->IsSceneItem(), "[EntityOutlinerView::OnGenerateRow]: Unknown item type encountered.");

			createInfo.Icon = ICON_FA_SITEMAP;
			createInfo.Name = item->pScene->GetName();
			createInfo.Type = "Scene";
			createInfo.IsSelected = m_SceneItemSelected;
			
			EntityManager& entityManager = item->pScene->GetEntityManager();
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

		m_SuspendNotifications = false;

		return pRow;
	}

	void EntityOutlinerView::OnGetChildren(const Ref<OutlinerListItem>& pParent, std::vector<Ref<OutlinerListItem>>& outChildren) noexcept
	{
		outChildren = pParent->Children;
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

	void EntityOutlinerView::OnRender() noexcept
	{
		PROFILE_FUNC;
		m_pMainBox->Render();
	}

	void EntityOutlinerView::OnRenameSelection() noexcept
	{

	}

	const std::vector<Ref<OutlinerListItem>>* EntityOutlinerView::OnRequestSource() noexcept
	{
		//TODO: A Sorting mechanism a bit more advanced, supporting ascending and descending strings.

		auto&& RecursiveSort = [&](auto&& Self, std::vector<Ref<OutlinerListItem>>& items) -> void
			{
				auto&& GetItemRank = [](const Ref<OutlinerListItem>& pItem) noexcept -> int 
					{
						if (pItem->IsEntityItem())
							return 2;
						else if (pItem->IsFilterItem())
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

						return GetItemName(pItemA) < GetItemName(pItemB);
					});

				for (const auto& item : items)
					Self(Self, item->Children);
			};

		RecursiveSort(RecursiveSort, m_ListItems);

		return &m_ListItems;
	}

	void EntityOutlinerView::OnSceneChanged(Scene* pScene) noexcept
	{
		m_EntityToItemMap.clear();
		m_FilterToItemMap.clear();
		m_ListItems.clear();
		m_SelectedFilters.clear();
		m_SceneItemSelected = false;

		m_pOutlinerTreeView->RequestTreeRefresh();

		if (!pScene)
			return;

		pScene->OnEntityCreated.Connect(this, &EntityOutlinerView::OnEntityCreated);
		pScene->OnEntityDestroyed.Connect(this, &EntityOutlinerView::OnEntityDestroyed);
		pScene->OnEntityAttached.Connect(this, &EntityOutlinerView::OnEntityAttached);
		pScene->OnEntityDetached.Connect(this, &EntityOutlinerView::OnEntityDetached);
		pScene->OnEntityVisibilityChanged.Connect(this, &EntityOutlinerView::OnEntityVisibilityChanged);

		m_ListItems.push_back(CreateSceneListItem(pScene));

		pScene->GetEntityManager().Collect<IDComponent, RootComponent>().Do([this](entity e)
			{
				m_ListItems[0]->Children.push_back(CreateEntityListItem(e));
			});
	}

	void EntityOutlinerView::OnSearchTextChanged(const char* pText) noexcept
	{
		m_SuspendNotifications = true;

		m_pOutlinerTreeView->ClearSelection();
		m_pOutlinerTreeView->ClearHightlightedItems();
		m_ListItems.clear();
		m_EntityToItemMap.clear();
		m_FilterToItemMap.clear();
		
		m_pFilter->SetFilterText(pText);

		const bool containsEntityLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetEntityTypeAsString(), ETextFilterTextComparisonMode::Partial);
		const bool containsFilterLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetFilterTypeAsString(), ETextFilterTextComparisonMode::Partial);
		const bool containsSceneLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetSceneTypeAsString(), ETextFilterTextComparisonMode::Partial);

		Scene* pScene = m_pEditor->GetActiveScene();

		EntityManager& mgr = pScene->GetEntityManager();
		const UniquePtr<Selection>& pSelection = m_pEditor->GetSelection();
		const UniquePtr<EntityFiltersManager>& pFilterManager = m_pEditor->GetEntityFiltersManager();

		auto&& RecursivelyTestAndAddEntityItem = [&](auto&& Self, entity e) -> void
			{
				bool added = false;

				if (containsEntityLabel || m_pFilter->TestTextFilter(mgr.Get<NameComponent>(e).Name, ETextFilterTextComparisonMode::Partial))
				{
					Ref<OutlinerListItem> pEntityItem = CreateEntityListItem(e);

					if (pSelection->IsEntitySelected(e))
						m_pOutlinerTreeView->SetItemSelection(pEntityItem, ESelectionType::Selected);

					m_pOutlinerTreeView->SetItemHighlighted(pEntityItem, true);

					if (mgr.Has<IsChildComponent>(e))
						m_EntityToItemMap[mgr.Get<IsChildComponent>(e).Parent]->Children.push_back(pEntityItem);
					else if (EntityFilter* pFilter = pFilterManager->GetFilterContainingEntity(e))
					{
						pFilterManager->ForEachFilterWithDescendantObject(pFilter, true, [](EntityFilter* pCurrentFilter)
							{
								//TODO... Should probably begin with this type of recursive function but for root filters first!!!
							});
					}
					else
					{
						if (m_ListItems.empty())
							m_ListItems.push_back(CreateSceneListItem(pScene));

						m_ListItems[0]->Children.push_back(pEntityItem);
					}

					added = true;
				}
				else
				{
					const std::vector<entity> descendants = m_pEditor->GetActiveScene()->GetAllEntityDescendants(e);
					if (std::any_of(descendants.begin(), descendants.end(), [&](entity descendant)
						{
							return m_pFilter->TestTextFilter(mgr.Get<NameComponent>(descendant).Name, ETextFilterTextComparisonMode::Partial);
						}
						))
					{
						Ref<OutlinerListItem> pEntityItem = CreateEntityListItem(e);

						if (pSelection->IsEntitySelected(e))
							m_pOutlinerTreeView->SetItemSelection(pEntityItem, ESelectionType::Selected);

						if (mgr.Has<IsChildComponent>(e))
							m_EntityToItemMap[mgr.Get<IsChildComponent>(e).Parent]->Children.push_back(pEntityItem);
						else
						{
							if (m_ListItems.empty())
								m_ListItems.push_back(CreateSceneListItem(pScene));

							m_ListItems[0]->Children.push_back(pEntityItem);
						}

						added = true;
					}
				}

				if (added)
				{
					const std::vector<entity> children = m_pEditor->GetActiveScene()->GetEntityChildren(e);
					for (auto child : children)
						Self(Self, child);
				}
			};

		mgr.Collect<RootComponent>().Do([&](entity e) { RecursivelyTestAndAddEntityItem(RecursivelyTestAndAddEntityItem, e); });

		if ((m_pFilter->TestTextFilter(pScene->GetName(), ETextFilterTextComparisonMode::Partial) || containsSceneLabel))
		{
			if (m_ListItems.empty())
				m_ListItems.push_back(CreateSceneListItem(pScene));

			m_pOutlinerTreeView->SetItemHighlighted(m_ListItems[0], true);
		}

		m_SuspendNotifications = false;
		m_pOutlinerTreeView->RequestTreeRefresh();
	}

	void EntityOutlinerView::OnSearchTextCommitted(const char* pText, ETextCommitType commitType) noexcept
	{
		if (commitType != ETextCommitType::OnEnter)
			return;

		if (std::strlen(pText) == 0u)
			return;

		m_pOutlinerTreeView->ClearSelection();

		const bool containsEntityLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetEntityTypeAsString(), ETextFilterTextComparisonMode::Partial);

		EntityManager& mgr = m_pEditor->GetActiveScene()->GetEntityManager();

		auto&& RecursiveSelectEligibleItems = [&](auto&& Self, const std::vector<Ref<OutlinerListItem>>& items) -> void
			{
				for (size_t i = 0u; i < items.size(); ++i)
				{
					const bool isEntityItem = items[i]->IsEntityItem();

					if (!m_pOutlinerTreeView->IsItemSelected(items[i]) && m_pFilter->TestTextFilter(isEntityItem ? mgr.Get<NameComponent>(items[i]->Entity).Name : "?", ETextFilterTextComparisonMode::Partial))
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

		m_SuspendNotifications = true;

		const UniquePtr<Selection>& pSelection = m_pEditor->GetSelection();

		OutlinerTableRow* pOutlinerTableRow = nullptr;
		Button* pVisibilityButton = nullptr;

		if (m_pOutlinerTreeView->IsItemVisible(item))
		{
			pOutlinerTableRow = static_cast<OutlinerTableRow*>(m_pOutlinerTreeView->GetRowWidget(item).Get());
			pVisibilityButton = pOutlinerTableRow->GetVisibilityButton();
		}

		if (selectionType == ESelectionType::Selected)
		{
			if (item->IsEntityItem())
				pSelection->SelectEntity(item->Entity);
			else if (item->IsFilterItem())
				m_SelectedFilters.insert(item->pFilter);
			else if (item->IsSceneItem())
				m_SceneItemSelected = true;

			if (pVisibilityButton)
			{
				pVisibilityButton->SetIsVisible(true);
				pVisibilityButton->SetAlpha(1.0f);
			}
		}
		else
		{
			if (item->IsEntityItem())
				pSelection->DeselectEntity(item->Entity);
			else if (item->IsFilterItem())
				m_SelectedFilters.erase(item->pFilter);
			else if (item->IsSceneItem())
				m_SceneItemSelected = false;

			if (pOutlinerTableRow && !pOutlinerTableRow->IsHovered())
			{
				pVisibilityButton->SetIsVisible(false);
				pVisibilityButton->SetAlpha(0.7f);
			}
		}

		m_SuspendNotifications = false;
	}

	void EntityOutlinerView::OnSelectionChangedExternally(entity e, ESelectionState selectionState) noexcept
	{
		if (m_SuspendNotifications)
			return;

		m_SuspendNotifications = true;
		m_pOutlinerTreeView->SetItemSelection(m_EntityToItemMap[e], (ESelectionType)selectionState);
		m_SuspendNotifications = false;
	}

	void EntityOutlinerView::OnVisibilityButtonClicked(Button* pButton, Ref<OutlinerListItem> pItem) noexcept
	{
		const bool isVisible = pButton->GetText() == ICON_FA_EYE;

		if (pItem->IsEntityItem())
			m_pEditor->GetActiveScene()->SetEntityVisibleInGame(pItem->Entity, !isVisible);
		else if (pItem->IsFilterItem())
		{
			const UniquePtr<EntityFiltersManager>& pFiltersManager = m_pEditor->GetEntityFiltersManager();
			Scene* pScene = m_pEditor->GetActiveScene();
			
			pFiltersManager->ForEachFilterWithRootObject(pItem->pFilter, true, [&](EntityFilter* pFilter)
				{
					const std::unordered_set<entity>& entitiesInFilter = pFilter->GetEntities();

					for (entity e : entitiesInFilter)
						pScene->SetEntityVisibleInGame(e, !isVisible);

					return true;
				});
		}
		else
		{
			RLS_ASSERT(pItem->IsSceneItem(), "[EntityOutlinerView::OnVisibilityButtonClicked]: Unknown item type encountered!");

			Scene* pScene = pItem->pScene;
			pScene->GetEntityManager().Collect<IDComponent>().Do([pScene, isVisible](entity e)
				{
					pScene->SetEntityVisibleInGame(e, !isVisible);
				});
		}
	
		if (!m_pOutlinerTreeView->IsItemSelected(pItem))
			return;

		std::vector<Ref<OutlinerListItem>> selectedItems;
		m_pOutlinerTreeView->GetSelectedItems(selectedItems);

		std::erase_if(selectedItems, [pItem](const Ref<OutlinerListItem>& pCurrentItem) { return pCurrentItem == pItem; });

		const UniquePtr<EntityFiltersManager>& pFiltersManager = m_pEditor->GetEntityFiltersManager();
		Scene* pScene = m_pEditor->GetActiveScene();

		for (const auto& pItem : selectedItems)
		{
			if (pItem->IsEntityItem())
				pScene->SetEntityVisibleInGame(pItem->Entity, !isVisible);
			else if (pItem->IsFilterItem())
			{
				const UniquePtr<EntityFiltersManager>& pFiltersManager = m_pEditor->GetEntityFiltersManager();
				Scene* pScene = m_pEditor->GetActiveScene();

				pFiltersManager->ForEachFilterWithRootObject(pItem->pFilter, true, [&](EntityFilter* pFilter)
					{
						const std::unordered_set<entity>& entitiesInFilter = pFilter->GetEntities();

						for (entity e : entitiesInFilter)
							pScene->SetEntityVisibleInGame(e, !isVisible);

						return true;
					});
			}
			else
			{
				RLS_ASSERT(pItem->IsSceneItem(), "[EntityOutlinerView::OnVisibilityButtonClicked]: Unknown item type encountered!");

				Scene* pScene = pItem->pScene;
				pScene->GetEntityManager().Collect<IDComponent>().Do([pScene, isVisible](entity e)
					{
						pScene->SetEntityVisibleInGame(e, !isVisible);
					});

				return;
			}
		}
	}
}
