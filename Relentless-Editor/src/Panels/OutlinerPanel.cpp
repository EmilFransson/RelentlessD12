// #include "OutlinerPanel.h"
// 
// #include "../Core/Editor.h"
// 
// namespace Relentless
// {
// 	namespace OutlinerPanel_private
// 	{
// 		[[nodiscard]] OutlinerTreeItem* AsOutlinerTreeItem(const std::shared_ptr<TreeItem>& pTreeItem)
// 		{
// 			return static_cast<OutlinerTreeItem*>(pTreeItem.get());
// 		}
// 
// 		[[nodiscard]] OutlinerEntityTreeItem* AsOutlinerEntityTreeItem(OutlinerTreeItem* pOutlinerTreeItem)
// 		{
// 			return static_cast<OutlinerEntityTreeItem*>(pOutlinerTreeItem);
// 		}
// 
// 		[[nodiscard]] OutlinerSceneTreeItem* AsOutlinerSceneTreeItem(OutlinerTreeItem* pOutlinerTreeItem)
// 		{
// 			return static_cast<OutlinerSceneTreeItem*>(pOutlinerTreeItem);
// 		}
// 
// 		[[nodiscard]] OutlinerFilterTreeItem* AsOutlinerFilterTreeItem(OutlinerTreeItem* pOutlinerTreeItem)
// 		{
// 			return static_cast<OutlinerFilterTreeItem*>(pOutlinerTreeItem);
// 		}
// 
// 		enum class SelectionMode : uint8_t { Single, Toggle, Range };
// 
// 		[[nodiscard]] SelectionMode GetSelectionMode() noexcept
// 		{
// 			if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
// 				return SelectionMode::Toggle;
// 			else if (Keyboard::IsKeyDown(RLS_Key::LShift))
// 				return SelectionMode::Range;
// 			else
// 				return SelectionMode::Single;
// 		}
// 	}
// 
// 	OutlinerPanel::OutlinerPanel(Editor* pEditor) noexcept
// 		: m_pEditor{ pEditor }
// 	{
// 		pEditor->OnSceneChanged.Connect(this, &OutlinerPanel::OnEditorSceneChanged);
// 		pEditor->GetSelection()->OnSelectionChanged.Connect(this, &OutlinerPanel::OnEntitySelectionChangedFromEditor);
// 
// 		if (const UniquePtr<EntityFiltersManager>& pEntityFiltersManager = m_pEditor->GetEntityFiltersManager())
// 		{
// 			//pEntityFiltersManager->OnFilterCreated.Connect(this, &OutlinerPanel::OnEntityFilterCreated);
// 			//pEntityFiltersManager->OnFilterDestroyed.Connect(this, &OutlinerPanel::OnEntityFilterDestroyed);
// 			//pEntityFiltersManager->OnEntitySetToFilter.Connect(this, &OutlinerPanel::OnEntitySetToFilter);
// 			//pEntityFiltersManager->OnEntityRemovedFromFilter.Connect(this, &OutlinerPanel::OnEntityRemovedFromFilter);
// 			//pEntityFiltersManager->OnFilterReattached.Connect(this, &OutlinerPanel::OnEntityFilterReattached);
// 		}
// 		
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\showicon.rasset"), m_ShowEntityTextureIconHandle), "Core engine icon missing.");
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\hideicon.rasset"), m_HideEntityTextureIconHandle), "Core engine icon missing.");
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\outlinerentityicon.rasset"), m_EntityTextureIconHandle), "Core engine icon missing.");
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\outlinersceneicon.rasset"), m_SceneTextureIconHandle), "Core engine icon missing.");
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\check.rasset"), m_CheckTextureIconHandle), "Core engine icon missing.");
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\not_allowed.rasset"), m_NotAllowedTextureIconHandle), "Core engine icon missing.");
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\entityfilteropen.rasset"), m_EntityFilterOpenTextureIconHandle), "Core engine icon missing.");
// 		//RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\entityfilterclosed.rasset"), m_EntityFilterClosedTextureIconHandle), "Core engine icon missing.");
// 		
// 		std::vector<AssetImportTask> importTasks;
// 		importTasks.reserve(8);
// 
// 		auto&& CreateUIImportTask = [&importTasks](const Path& srcPath, AssetHandle& handleToSet)
// 			{
// 				Ref<TextureFactory> pTextureFactory = RLS_NEW TextureFactory();
// 				pTextureFactory->SetImportAsSRGB(true);
// 				pTextureFactory->OnDone.Connect([&](const ImportedAsset& asset, bool success)
// 					{
// 						RLS_VERIFY(success, "[OutlinerPanel] Error importing UI texture asset.");
// 						handleToSet = asset.Handle;
// 					});
// 
// 				AssetImportTask& importTask = importTasks.emplace_back();
// 				importTask.FilePath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, srcPath);
// 				importTask.pFactory = pTextureFactory;
// 			};
// 
// 		CreateUIImportTask("Textures\\Icons\\showicon.png", m_ShowEntityTextureIconHandle);
// 		CreateUIImportTask("Textures\\Icons\\hideicon.png", m_HideEntityTextureIconHandle);
// 		CreateUIImportTask("Textures\\Icons\\png_icbfo.png", m_EntityTextureIconHandle);
// 		CreateUIImportTask("Textures\\Icons\\png_o2wr5.png", m_SceneTextureIconHandle);
// 		CreateUIImportTask("Textures\\Icons\\check.png", m_CheckTextureIconHandle);
// 		CreateUIImportTask("Textures\\Icons\\not_allowed.png", m_NotAllowedTextureIconHandle);
// 		CreateUIImportTask("Textures\\Icons\\entityfilteropen.png", m_EntityFilterOpenTextureIconHandle);
// 		CreateUIImportTask("Textures\\Icons\\entityfilterclosed.png", m_EntityFilterClosedTextureIconHandle);
// 
// 		Importer::RequestAsyncLoad(importTasks).Wait();
// 
// 		SetupOutlinerTable();
// 	}
// 
// 	OutlinerPanel::~OutlinerPanel() noexcept
// 	{
// 		if (m_pEditor)
// 		{
// 			m_pEditor->OnSceneChanged.Detach(this);
// 
// 			if (const UniquePtr<Selection>& pSelection = m_pEditor->GetSelection())
// 				pSelection->OnSelectionChanged.Detach(this);
// 
// 			if (const UniquePtr<EntityFiltersManager>& pEntityFiltersManager = m_pEditor->GetEntityFiltersManager())
// 			{
// 				pEntityFiltersManager->OnFilterCreated.Detach(this);
// 				pEntityFiltersManager->OnFilterDestroyed.Detach(this);
// 				pEntityFiltersManager->OnEntitySetToFilter.Detach(this);
// 				pEntityFiltersManager->OnEntityRemovedFromFilter.Detach(this);
// 				pEntityFiltersManager->OnFilterReattached.Detach(this);
// 			}
// 			
// 		}
// 
// 		if (m_pScene)
// 		{
// 			m_pScene->OnEntityCreated.Detach(this);
// 			m_pScene->OnEntityPreDestroyed.Detach(this);
// 			m_pScene->OnEntityAttached.Detach(this);
// 			m_pScene->OnEntityDetached.Detach(this);
// 			m_pScene->OnEntityVisibilityChanged.Detach(this);
// 		}
// 
// 		if (m_pOutliner)
// 			m_pOutliner->OnFocusChanged.Detach(this);
// 
// 		if (m_pTreeInteraction)
// 		{
// 			m_pTreeInteraction->OnItemClicked.Detach(this);
// 			m_pTreeInteraction->OnItemHovered.Detach(this);
// 			m_pTreeInteraction->OnMouseEnterItemRow.Detach(this);
// 			m_pTreeInteraction->OnMouseExitItemRow.Detach(this);
// 			m_pTreeInteraction->OnMouseReleasedOnItem.Detach(this);
// 		}
// 
// 		if (m_pDragDropBehavior)
// 		{
// 			m_pDragDropBehavior->OnDragOver.Detach(this);
// 			m_pDragDropBehavior->OnDrop.Detach(this);
// 		}
// 	}
// 
// 	void OutlinerPanel::OnImGuiRender(const bool show) noexcept
// 	{
// 		if (!show)
// 			return;
// 		
// 		PROFILE_FUNC;
// 		
// 		m_DraggedOnValidTargetThisFrame = false;
// 		m_DragDropTooltip.clear();
// 		m_DragDropTooltipIcon = NULL_HANDLE;
// 
// 		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
// 		ImGui::Begin("Outliner");
// 		ImGui::PopStyleVar();
// 		
// 		constexpr float bannerHeight = 40.0f;
// 
// 		{
// 			ImGui::BeginChild("Outliner SearchBar Child", ImVec2(0.0f, 50.0f));
// 			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
// 			//UI::SearchBar("OutlinerSearchBar", "Search...", true);
// 			ImGui::EndChild();
// 		}
// 
// 		{
// 			ImGui::BeginChild("Outliner Table Child", ImVec2(0, ImGui::GetContentRegionAvail().y - bannerHeight));
// 			m_pOutliner->Draw();
// 
// 			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !m_pOutliner->IsHovered())
// 				DeselectAllTreeItems();
// 
// 			ImGui::EndChild();
// 		}
// 
// 		{
// 			ImGui::BeginChild("Outliner Banner Child");
// 			
// 			const ImVec2 bannerStartPos = ImGui::GetCursorScreenPos();
// 			const ImVec2 availableSpace = ImGui::GetContentRegionAvail();
// 			const ImVec2 bannerEndPos = ImVec2(bannerStartPos.x + availableSpace.x, bannerStartPos.y + availableSpace.y);
// 
// 			ImDrawList* pDrawList = ImGui::GetWindowDrawList();
// 			pDrawList->AddRectFilled(bannerStartPos, bannerEndPos, ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)));
// 
// 			const uint32_t numEntities = m_pScene->GetEntityManager().GetEntityAliveCount();
// 			const uint32_t selectedEntities = m_pEditor->GetSelection()->GetSelectedEntityCount();
// 
// 			std::string bannerLabel = std::format("{} Entities", numEntities);
// 			if (selectedEntities > 0)
// 				bannerLabel += std::format(" ({} selected)", selectedEntities);
// 
// 			constexpr float bannerLabelOffsetFromBorder = 10.0f;
// 			const float bannerLabelXPos = bannerStartPos.x + bannerLabelOffsetFromBorder;
// 
// 			const float bannerLabelHeight = UI::Utility::CalculateTextHeight(bannerLabel);
// 			const float bannerLabelYPos = bannerStartPos.y + (bannerHeight / 2.0f) - (bannerLabelHeight / 2.0f);
// 
// 			pDrawList->AddText(ImVec2(bannerLabelXPos, bannerLabelYPos), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), bannerLabel.c_str());
// 
// 			ImGui::EndChild();
// 		}
// 
// 		ImGui::End();
// 
// 		if (m_pDragDropBehavior->IsActive())
// 		{
// 			if (!m_DragDropTooltip.empty())
// 			{
// 				ImGui::BeginTooltip();
// 				
// 				const Ref<Texture> pDragDropTexture = AssetManager::Get<Texture>(m_DragDropTooltipIcon);
// 				ImGui::Image((ImTextureID)pDragDropTexture->GetSRV()->GetGPUHandle().ptr, ImVec2(static_cast<float>(pDragDropTexture->GetWidth()), static_cast<float>(pDragDropTexture->GetHeight())));
// 				ImGui::SameLine();
// 				ImGui::Text(m_DragDropTooltip.c_str());
// 				
// 				ImGui::EndTooltip();
// 			}
// 		}
// 	}
// 
// 	void OutlinerPanel::SelectAll() noexcept
// 	{
// 		const std::shared_ptr<TreeItem>& pSceneTreeItem = m_pOutliner->GetRootEntries().front();
// 		std::vector<TreeItem*> allTreeItems = pSceneTreeItem->GetDescendants();
// 		allTreeItems.push_back(pSceneTreeItem.get());
// 
// 		for (TreeItem* pTreeItem : allTreeItems)
// 		{
// 			if (!IsTreeItemSelected(pTreeItem))
// 				SelectTreeItem(static_cast<OutlinerTreeItem*>(pTreeItem));
// 		}
// 	}
// 
// 	void OutlinerPanel::SelectAllExpanded() noexcept
// 	{
// 		const std::shared_ptr<TreeItem>& pSceneTreeItem = m_pOutliner->GetRootEntries().front();
// 		std::vector<TreeItem*> allTreeItems = pSceneTreeItem->GetDescendants();
// 		allTreeItems.push_back(pSceneTreeItem.get());
// 
// 		for (TreeItem* pTreeItem : allTreeItems)
// 		{
// 			if (!pTreeItem->IsVisible())
// 				continue;
// 
// 			if (!IsTreeItemSelected(pTreeItem))
// 				SelectTreeItem(static_cast<OutlinerTreeItem*>(pTreeItem));
// 		}
// 	}
// 
// 	void OutlinerPanel::DeselectNonEntityItems() noexcept
// 	{
// 		std::vector<OutlinerTreeItem*> toDeselect;
// 		toDeselect.reserve(m_SelectedEntityFilters.size());
// 
// 		for (OutlinerTreeItem* pFilterItem : m_SelectedEntityFilters)
// 			toDeselect.push_back(pFilterItem);
// 
// 		if (!m_pOutliner->GetRootEntries().empty() && IsTreeItemSelected(m_pOutliner->GetRootEntries().front().get()))
// 			toDeselect.push_back(static_cast<OutlinerTreeItem*>(m_pOutliner->GetRootEntries().front().get()));
// 
// 		for (OutlinerTreeItem* pFilterItemToDeselect : toDeselect)
// 			DeselectTreeItem(pFilterItemToDeselect);
// 	}
// 
// 	void OutlinerPanel::OnDeleteKeyPressed() noexcept
// 	{
// 		const std::vector<std::shared_ptr<TreeItem>> selectedTreeItems = GetAllSelectedTreeItems();
// 		
// 		//Delete entities first and filters last:
// 		std::vector<OutlinerEntityTreeItem*> selectedEntityTreeItems;
// 		std::vector<OutlinerFilterTreeItem*> selectedFilterTreeItems;
// 
// 		for (auto& pTreeItem : selectedTreeItems)
// 		{
// 			OutlinerTreeItem* outlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);
// 			switch (outlinerTreeItem->GetType())
// 			{
// 			case ETreeItemType::Entity:
// 				selectedEntityTreeItems.push_back(OutlinerPanel_private::AsOutlinerEntityTreeItem(outlinerTreeItem));
// 				break;
// 			case ETreeItemType::Filter:
// 				selectedFilterTreeItems.push_back(OutlinerPanel_private::AsOutlinerFilterTreeItem(outlinerTreeItem));
// 				break;
// 			}
// 		
// 		}
// 
// 		EntityFiltersManager& filterManager = *m_pEditor->GetEntityFiltersManager();
// 		
// 		for (OutlinerEntityTreeItem* pEntityTreeItem : selectedEntityTreeItems)
// 		{
// 			std::vector<std::shared_ptr<TreeItem>> children = pEntityTreeItem->GetChildren();
// 
// 			TreeItem* pNonEntityAncestor = GetFirstNonEntityTreeItemAncestor(pEntityTreeItem);
// 
// 			m_pScene->DestroyEntity(pEntityTreeItem->GetEntityID());
// 		
// 			if (children.empty())
// 				return;
// 
// 			if (!pNonEntityAncestor)
// 				return;
// 
// 			OutlinerTreeItem* pNonEntityOutlinerTreeItem = static_cast<OutlinerTreeItem*>(pNonEntityAncestor);
// 			if (pNonEntityOutlinerTreeItem->GetType() == ETreeItemType::Filter)
// 			{
// 				OutlinerFilterTreeItem* pFilterTreeItem = OutlinerPanel_private::AsOutlinerFilterTreeItem(pNonEntityOutlinerTreeItem);
// 
// 				for (auto& child : children)
// 				{
// 					OutlinerTreeItem* pChildOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(child);
// 					if (pChildOutlinerTreeItem->GetType() != ETreeItemType::Entity)
// 						continue;
// 
// 					OutlinerEntityTreeItem* pEntityOutlinerTreeItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pChildOutlinerTreeItem);
// 					filterManager.SetEntityToFilter(pEntityOutlinerTreeItem->GetEntityID(), pFilterTreeItem->GetPath());
// 				}
// 			}
// 		}
// 
// 		for (OutlinerFilterTreeItem* pFilterTreeItem : selectedFilterTreeItems)
// 			filterManager.DestroyFilter(pFilterTreeItem->GetPath());
// 	}
// 
// 	bool OutlinerPanel::IsFocused() const noexcept
// 	{
// 		return m_pOutliner->IsFocused();
// 	}
// 
// 	void OutlinerPanel::OnEditorSceneChanged(Scene* pScene) noexcept
// 	{
// 		if (m_pScene)
// 		{
// 			m_pScene->OnEntityCreated.Detach(this);
// 			m_pScene->OnEntityPreDestroyed.Detach(this);
// 			m_pScene->OnEntityAttached.Detach(this);
// 			m_pScene->OnEntityDetached.Detach(this);
// 			m_pScene->OnEntityVisibilityChanged.Detach(this);
// 		}
// 
// 		m_pScene = pScene;
// 
// 		m_pScene->OnEntityCreated.Connect(this, &OutlinerPanel::OnEntityCreated);
// 		m_pScene->OnEntityPreDestroyed.Connect(this, &OutlinerPanel::OnEntityPreDestroyed);
// 		m_pScene->OnEntityAttached.Connect(this, &OutlinerPanel::OnEntityAttached);
// 		m_pScene->OnEntityDetached.Connect(this, &OutlinerPanel::OnEntityDetached);
// 		m_pScene->OnEntityVisibilityChanged.Connect(this, &OutlinerPanel::OnEntityVisibilityChanged);
// 
// 		m_pOutliner->RemoveAll();
// 
// 		m_pOutliner->AddEntry(CreateSceneTreeItem(m_pScene));
// 
// 		m_pScene->GetEntityManager().Collect<RootComponent>().Do([this](entity e)
// 			{
// 				std::function<void(entity entityID, const std::shared_ptr<OutlinerEntityTreeItem>& pParentOutlinerEntityTreeItem)> RecursiveAddEntityTreeItem
// 					 = [&](entity entityID, const std::shared_ptr<OutlinerEntityTreeItem>& pParentOutlinerEntityTreeItem)
// 				{
// 					const std::shared_ptr<OutlinerEntityTreeItem> pNewEntityTreeItem = CreateEntityTreeItem(entityID);
// 					
// 					if (pParentOutlinerEntityTreeItem)
// 						pParentOutlinerEntityTreeItem->AddChild(pNewEntityTreeItem);
// 					else
// 						AddToRoot(pNewEntityTreeItem);
// 
// 					if (m_pScene->GetEntityManager().Has<ParentComponent>(entityID))
// 					{
// 						const std::vector<entity>& children = m_pScene->GetEntityManager().Get<ParentComponent>(entityID).Children;
// 						for (auto child : children)
// 							RecursiveAddEntityTreeItem(child, pNewEntityTreeItem);
// 					}
// 				};
// 
// 				RecursiveAddEntityTreeItem(e, nullptr);
// 			});
// 	}
// 
// 	void OutlinerPanel::OnEntityCreated(entity e) noexcept
// 	{
// 		AddToRoot(CreateEntityTreeItem(e));
// 	}
// 
// 	void OutlinerPanel::OnEntityPreDestroyed(entity e) noexcept
// 	{
// 		TreeItem* pParentTreeItem = m_EntityToTreeItemMap[e]->GetParent();
// 		const std::vector<std::shared_ptr<TreeItem>>& children = m_EntityToTreeItemMap[e]->GetChildren();
// 		
// 		if (pParentTreeItem)
// 		{
// 			pParentTreeItem->RemoveChild(m_EntityToTreeItemMap[e]);
// 
// 			for (auto& child : children)
// 				pParentTreeItem->AddChild(child);
// 		}
// 		else
// 		{
// 			m_pOutliner->RemoveEntry(m_EntityToTreeItemMap[e]);
// 
// 			for (auto& child : children)
// 			{
// 				child->RemoveParent();
// 				m_pOutliner->AddEntry(child);
// 			}
// 		}
// 
// 		m_EntityToTreeItemMap.erase(e);
// 	}
// 
// 	//It could be that a re-attachment has occured (detached -> attached) in which
// 	//case the parent-child relation already has been resolved. This is thus checked.
// 	void OutlinerPanel::OnEntityAttached(entity child, entity parent) noexcept
// 	{
// 		std::shared_ptr<OutlinerTreeItem> pChildTreeItem = m_EntityToTreeItemMap[child];
// 		std::shared_ptr<OutlinerTreeItem> pParentTreeItem = m_EntityToTreeItemMap[parent];
// 		
// 		if (pChildTreeItem->GetParent() == pParentTreeItem.get())
// 			return;
// 
// 		OutlinerSceneTreeItem* pSceneTreeItem = GetSceneTreeItem();
// 		if (pChildTreeItem->GetParent() == pSceneTreeItem)
// 			pSceneTreeItem->RemoveChild(pChildTreeItem);
// 		
// 		pParentTreeItem->AddChild(pChildTreeItem);
// 	}
// 
// 	void OutlinerPanel::OnEntityDetached(entity child, entity parent) noexcept
// 	{
// 		if (m_SuspendNotifications)
// 			return;
// 
// 		std::shared_ptr<OutlinerTreeItem> pChildTreeItem = m_EntityToTreeItemMap[child];
// 		std::shared_ptr<OutlinerTreeItem> pParentTreeItem = m_EntityToTreeItemMap[parent];
// 
// 		pParentTreeItem->RemoveChild(pChildTreeItem);
// 		GetSceneTreeItem()->AddChild(pChildTreeItem);
// 	}
// 
// 	void OutlinerPanel::OnEntitySelectionChangedFromEditor(entity e, ESelectionState selectionState) noexcept
// 	{
// 		if (!m_EntityToTreeItemMap.contains(e))
// 			return;
// 
// 		std::shared_ptr<OutlinerTreeItem> pEntityTreeItem = m_EntityToTreeItemMap[e];
// 
// 		switch (selectionState)
// 		{
// 		case ESelectionState::Selected:
// 		{
// 			pEntityTreeItem->SetBackgroundColor(m_pOutliner->IsFocused() ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
// 			pEntityTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
// 
// 			std::vector<TreeItem*> ancestors = pEntityTreeItem->GetAncestors();
// 			for (TreeItem* ancestor : ancestors)
// 			{
// 				if (!IsTreeItemSelected(ancestor))
// 					ancestor->SetBackgroundColor(TreeDefaultColors::RowAncestorToSelectedColor);
// 			}
// 
// 			break;
// 		}
// 		case ESelectionState::Deselected:
// 		{
// 			pEntityTreeItem->ResetBackgroundColor();
// 			const ImVec4 iconTint = pEntityTreeItem->IsVisible() ? ImVec4(1.0f, 1.0f, 1.0f, 0.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
// 			pEntityTreeItem->SetIconTint(0u, iconTint);
// 
// 			std::vector<TreeItem*> ancestors = pEntityTreeItem->GetAncestors();
// 			for (TreeItem* ancestor : ancestors)
// 			{
// 				std::vector<TreeItem*> ancestorsDescendants = ancestor->GetDescendants();
// 				const bool noneIsSelected = std::all_of(ancestorsDescendants.begin(), ancestorsDescendants.end(), [this](TreeItem* pCurrentTreeItem)
// 					{
// 						return IsTreeItemSelected(pCurrentTreeItem) == false;
// 					});
// 
// 				if (noneIsSelected)
// 					ancestor->ResetBackgroundColor();
// 			}
// 
// 			break;
// 		}
// 		default:
// 		{
// 			RLS_ASSERT(false, "Unreachable");
// 			break;
// 		}
// 		}
// 	}
// 
// 	void OutlinerPanel::OnEntityVisibilityChanged(entity e, bool visibilityState) noexcept
// 	{
// 		const bool isSelected = m_pEditor->GetSelection()->IsEntitySelected(e);
// 
// 		m_EntityToTreeItemMap[e]->SetVisibility(visibilityState);
// 		m_EntityToTreeItemMap[e]->SetVisibilityIcon(visibilityState ? m_ShowEntityTextureIconHandle : m_HideEntityTextureIconHandle);
// 		m_EntityToTreeItemMap[e]->SetIconTint(0u, !visibilityState ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f) : (visibilityState && isSelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
// 	}
// 
// 	void OutlinerPanel::OnOutlinerTreeFocusChanged(bool isFocused) noexcept
// 	{
// 		const std::vector<entity>& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();
// 		for (entity selectedEntity : selectedEntities)
// 			m_EntityToTreeItemMap[selectedEntity]->SetBackgroundColor(isFocused ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
// 
// 		if (m_SceneTreeItemSelected)
// 			GetSceneTreeItem()->SetBackgroundColor(isFocused ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
// 
// 		for (OutlinerTreeItem* pFilterTreeItem : m_SelectedEntityFilters)
// 			pFilterTreeItem->SetBackgroundColor(isFocused ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
// 	}
// 
// 	void OutlinerPanel::SetupOutlinerTable() noexcept
// 	{
// 		m_pTreeInteraction = std::make_shared<TreeInteraction>();
// 		m_pTreeInteraction->OnItemClicked.Connect(this, &OutlinerPanel::OnTreeItemClicked);
// 		m_pTreeInteraction->OnItemHovered.Connect(this, &OutlinerPanel::OnTreeItemHovered);
// 		m_pTreeInteraction->OnMouseEnterItemRow.Connect(this, &OutlinerPanel::OnMouseEnterTreeItemRow);
// 		m_pTreeInteraction->OnMouseExitItemRow.Connect(this, &OutlinerPanel::OnMouseExitTreeItemRow);
// 		m_pTreeInteraction->OnMouseReleasedOnItem.Connect(this, &OutlinerPanel::OnMouseReleasedOnTreeItem);
// 
// 		std::shared_ptr<TreeStyle> pTreeStyle = std::make_shared<TreeStyle>();
// 		pTreeStyle->SetUseAlternatingRowColors(true);
// 
// 		std::shared_ptr<TreeDataView> pDataView = std::make_shared<TreeDataView>();
// 
// 		m_pDragDropBehavior = std::make_shared<DragDropBehavior>("OutlinerTreeDragDrop");
// 		m_pDragDropBehavior->OnDragOver.Connect(this, &OutlinerPanel::OnDragOver);
// 		m_pDragDropBehavior->OnDrop.Connect(this, &OutlinerPanel::OnDrop);
// 
// 		pDataView->pTreeInteraction = m_pTreeInteraction;
// 		pDataView->pTreeStyle = pTreeStyle;
// 		pDataView->pDragDropBehavior = m_pDragDropBehavior;
// 
// 		m_pOutliner = std::make_shared<Tree>("Outliner", pDataView);
// 		m_pOutliner->OnFocusChanged.Connect(this, &OutlinerPanel::OnOutlinerTreeFocusChanged);
// 
// 		{
// 			Tree::ColumnProperties column;
// 			column.Label = "";
// 			column.HeaderTooltip = "Visibility";
// 			column.HeaderIcon.IconTextureHandle = m_ShowEntityTextureIconHandle;
// 			column.HeaderIcon.Tint = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
// 			column.HeaderIcon.SizeWeight = 0.6f;
// 			column.Flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort;
// 			column.DefaultWeight = 30.0f;
// 			column.AllowSelection = false;
// 			column.Alignment = UI::Alignment::Center;
// 			m_pOutliner->AddColumn(column);
// 		}
// 
// 		{
// 			Tree::ColumnProperties column;
// 			column.Label = "Item Label";
// 			column.HeaderTooltip = "Item Label";
// 			column.Flags = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort;
// 			column.IsTreeNode = true;
// 			column.Alignment = UI::Alignment::Left;
// 			m_pOutliner->AddColumn(column);
// 		}
// 
// 		{
// 			Tree::ColumnProperties column;
// 			column.Label = "Type";
// 			column.HeaderTooltip = "Displays the name of each entity's type";
// 			column.Flags = ImGuiTableColumnFlags_WidthStretch;
// 			m_pOutliner->AddColumn(column);
// 		}
// 
// 		m_pOutliner->SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Sortable);
// 	}
// 
// 	std::shared_ptr<OutlinerEntityTreeItem> OutlinerPanel::CreateEntityTreeItem(entity e) noexcept
// 	{
// 		TreeItemData data;
// 		data.ColumnLabels.resize(3);
// 
// 		data.ColumnLabels[0] = "";
// 		data.ColumnLabels[1] = m_pScene->GetEntityManager().Get<NameComponent>(e).Name;
// 		data.ColumnLabels[2] = "Entity";
// 
// 		data.ColumnIcons.resize(3);
// 		data.ColumnIcons[0].IconTextureHandle = m_ShowEntityTextureIconHandle;
// 		data.ColumnIcons[0].Tint = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
// 
// 		data.ColumnIcons[1].IconTextureHandle = m_EntityTextureIconHandle;
// 		data.ColumnIcons[1].Tint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
// 		data.ColumnIcons[1].SizeWeight = 0.7f;
// 
// 		data.ColumnStyles.resize(3);
// 		data.ColumnStyles[0].Alignment = UI::Alignment::Center;
// 
// 		data.UseDefaultItemColor = true;
// 
// 		std::shared_ptr<OutlinerEntityTreeItem> pEntityTreeItem = std::make_shared<OutlinerEntityTreeItem>(e);
// 		pEntityTreeItem->SetData(data);
// 
// 		m_EntityToTreeItemMap[e] = pEntityTreeItem;
// 
// 		return pEntityTreeItem;
// 	}
// 
// 	std::shared_ptr<OutlinerSceneTreeItem> OutlinerPanel::CreateSceneTreeItem(Scene* pScene) noexcept
// 	{
// 		TreeItemData data;
// 		data.ColumnLabels.resize(3);
// 
// 		data.ColumnLabels[0] = "";
// 		data.ColumnLabels[1] = pScene->GetName();
// 		data.ColumnLabels[2] = "Scene";
// 
// 		data.ColumnIcons.resize(3);
// 		data.ColumnIcons[0].IconTextureHandle = m_ShowEntityTextureIconHandle;
// 		data.ColumnIcons[0].Tint = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
// 
// 		data.ColumnIcons[1].IconTextureHandle = m_SceneTextureIconHandle;
// 		data.ColumnIcons[1].Tint = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
// 		data.ColumnIcons[1].SizeWeight = 0.7f;
// 
// 		data.ColumnStyles.resize(3);
// 		data.ColumnStyles[0].Alignment = UI::Alignment::Center;
// 
// 		data.UseDefaultItemColor = true;
// 
// 		std::shared_ptr<OutlinerSceneTreeItem> pSceneTreeItem = std::make_shared<OutlinerSceneTreeItem>();
// 		pSceneTreeItem->SetData(data);
// 
// 		return pSceneTreeItem;
// 	}
// 
// 	std::shared_ptr<OutlinerFilterTreeItem> OutlinerPanel::CreateFilterTreeItem(EntityFilter* pFilter) noexcept
// 	{
// 		TreeItemData data;
// 		data.ColumnLabels.resize(3);
// 
// 		data.ColumnLabels[0] = "";
// 		data.ColumnLabels[1] = pFilter->GetName();
// 		data.ColumnLabels[2] = "Filter";
// 
// 		data.ColumnIcons.resize(3);
// 		data.ColumnIcons[0].IconTextureHandle = m_ShowEntityTextureIconHandle;
// 		data.ColumnIcons[0].Tint = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
// 
// 		data.ColumnIcons[1].IconTextureHandle = pFilter->IsExpanded() ? m_EntityFilterOpenTextureIconHandle : m_EntityFilterClosedTextureIconHandle;
// 		data.ColumnIcons[1].Tint = ImVec4(0.55f, 0.6f, 0.6f, 1.0f);
// 		data.ColumnIcons[1].SizeWeight = 0.7f;
// 
// 		data.ColumnStyles.resize(3);
// 		data.ColumnStyles[0].Alignment = UI::Alignment::Center;
// 
// 		data.UseDefaultItemColor = true;
// 
// 		const std::string path = pFilter->GetPath();
// 
// 		std::shared_ptr<OutlinerFilterTreeItem> pEntityFilterTreeItem = std::make_shared<OutlinerFilterTreeItem>(path);
// 		pEntityFilterTreeItem->SetData(data);
// 
// 		m_FilterPathToTreeItemMap[path] = pEntityFilterTreeItem;
// 
// 		return pEntityFilterTreeItem;
// 	}
// 
// 	void OutlinerPanel::AddToRoot(const std::shared_ptr<TreeItem>& pTreeItem) noexcept
// 	{
// 		GetSceneTreeItem()->AddChild(pTreeItem);
// 	}
// 
// 	void OutlinerPanel::OnTreeItemClicked(std::shared_ptr<TreeItem> pTreeItem, uint32_t column, bool doubleClicked) noexcept
// 	{
// 		const EColumnType columnType = static_cast<EColumnType>(column);
// 
// 		OutlinerTreeItem* pItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);
// 		const ETreeItemType itemType = pItem->GetType();
// 
// 		if (doubleClicked && itemType == ETreeItemType::Filter)
// 		{
// 			if (columnType == EColumnType::Visibility)
// 				return;
// 
// 			OutlinerFilterTreeItem* pFilterTreeItem = OutlinerPanel_private::AsOutlinerFilterTreeItem(pItem);
// 			
// 			const bool newExpandState = !pFilterTreeItem->IsExpanded();
// 			pFilterTreeItem->SetExpanded(newExpandState);
// 			pFilterTreeItem->SetColumnIcon(newExpandState ? m_EntityFilterOpenTextureIconHandle : m_EntityFilterClosedTextureIconHandle, 1);
// 
// 			m_pEditor->GetEntityFiltersManager()->GetFilter(pFilterTreeItem->GetPath())->SetExpandedState(newExpandState);
// 		}
// 		else
// 		{
// 			if (columnType == EColumnType::Visibility)
// 			{
// 				const bool newVisibilityState = !pItem->IsVisible();
// 				
// 				std::vector<std::shared_ptr<TreeItem>> treeItemsToEditVisibilityFor = GetAllSelectedTreeItems();
// 				if (!IsTreeItemSelected(pItem))
// 					treeItemsToEditVisibilityFor.push_back(pTreeItem);
// 
// 				const bool anyIsScene = std::any_of(treeItemsToEditVisibilityFor.begin(), treeItemsToEditVisibilityFor.end(), [this](const std::shared_ptr<TreeItem>& pCurrentTreeItem)
// 					{
// 						//Scene always at outliner tree root:
// 						return pCurrentTreeItem == m_pOutliner->GetRootEntries().front();
// 					});
// 
// 				if (anyIsScene)
// 				{
// 					std::vector<std::shared_ptr<TreeItem>> allItems = m_pOutliner->GetDescendants(m_pOutliner->GetRootEntries().front());
// 					allItems.push_back(m_pOutliner->GetRootEntries().front());
// 					for (auto& treeItem : allItems)
// 					{
// 						OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(treeItem);
// 						const ETreeItemType type = pOutlinerTreeItem->GetType();
// 						switch (type)
// 						{
// 						case ETreeItemType::Entity:
// 						{
// 							const entity e = OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem)->GetEntityID();
// 							m_pScene->SetEntityVisibleInGame(e, newVisibilityState);
// 							break;
// 						}
// 						case ETreeItemType::Scene:
// 						case ETreeItemType::Filter:
// 						{
// 							pOutlinerTreeItem->SetVisibility(newVisibilityState);
// 							pOutlinerTreeItem->SetVisibilityIcon(newVisibilityState ? m_ShowEntityTextureIconHandle : m_HideEntityTextureIconHandle);
// 							pOutlinerTreeItem->SetIconTint(0u, !newVisibilityState ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f) : (newVisibilityState && m_SceneTreeItemSelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
// 							break;
// 						}
// 						}
// 					}
// 				}
// 				else
// 				{
// 					for (auto& treeItem : treeItemsToEditVisibilityFor)
// 					{
// 						OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(treeItem);
// 						const ETreeItemType type = pOutlinerTreeItem->GetType();
// 						switch (type)
// 						{
// 						case ETreeItemType::Entity:
// 						{
// 							const entity e = OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem)->GetEntityID();
// 							m_pScene->SetEntityVisibleInGame(e, newVisibilityState);
// 							break;
// 						}
// 						case ETreeItemType::Filter:
// 						{
// 							OutlinerFilterTreeItem* pFilterItem = OutlinerPanel_private::AsOutlinerFilterTreeItem(pOutlinerTreeItem);
// 							const bool isSelected = m_SelectedEntityFilters.contains(pFilterItem);
// 
// 							pFilterItem->SetVisibility(newVisibilityState);
// 							pFilterItem->SetVisibilityIcon(newVisibilityState ? m_ShowEntityTextureIconHandle : m_HideEntityTextureIconHandle);
// 							pFilterItem->SetIconTint(0u, !newVisibilityState  ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f) : (newVisibilityState && isSelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
// 
// 							std::vector<TreeItem*> descendants = pFilterItem->GetDescendants();
// 							for (TreeItem* descendant : descendants)
// 							{
// 								if (std::none_of(treeItemsToEditVisibilityFor.begin(), treeItemsToEditVisibilityFor.end(), [descendant](const std::shared_ptr<TreeItem>& pPossibleDescendant)
// 									{
// 										return descendant == pPossibleDescendant.get();
// 									}))
// 								{
// 									OutlinerTreeItem* pDescendantOutlinerTreeItem = static_cast<OutlinerTreeItem*>(descendant);
// 									switch (pDescendantOutlinerTreeItem->GetType())
// 									{
// 									case ETreeItemType::Filter:
// 									{
// 										pDescendantOutlinerTreeItem->SetVisibility(newVisibilityState);
// 										pDescendantOutlinerTreeItem->SetVisibilityIcon(newVisibilityState ? m_ShowEntityTextureIconHandle : m_HideEntityTextureIconHandle);
// 										pDescendantOutlinerTreeItem->SetIconTint(0u, !newVisibilityState ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f) : (newVisibilityState && isSelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
// 										break;
// 									}
// 									case ETreeItemType::Entity:
// 									{
// 										const entity e = OutlinerPanel_private::AsOutlinerEntityTreeItem(pDescendantOutlinerTreeItem)->GetEntityID();
// 										m_pScene->SetEntityVisibleInGame(e, newVisibilityState);
// 										break;
// 									}
// 									}
// 
// 								
// 								}
// 							}
// 
// 							break;
// 						}
// 						}
// 					}
// 				}
// 			}
// 			else
// 				DetermineAndIssueSelection(pItem, column);
// 		}
// 	}
// 
// 	void OutlinerPanel::OnTreeItemHovered(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept
// 	{
// 		const EColumnType columnType = static_cast<EColumnType>(column);
// 
// 		if (!m_pDragDropBehavior->IsActive())
// 		{
// 			switch (columnType)
// 			{
// 			case EColumnType::Visibility: UI::Utility::DrawTooltip("Toggles the visibility of this entry in the scene editor."); break;
// 			case EColumnType::Label: UI::Utility::DrawTooltip(pTreeItem->GetColumnLabel(column).c_str()); break;
// 			default: break;
// 			}
// 		}
// 
// 		if (!IsTreeItemSelected(pTreeItem.get()))
// 		{
// 			const ImVec4 visibilityIconTint = column == 0u ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
// 			pTreeItem->SetIconTint(0u, visibilityIconTint);
// 		}
// 	}
// 
// 	void OutlinerPanel::OnMouseEnterTreeItemRow(std::shared_ptr<TreeItem> pTreeItem, [[maybe_unused]] uint32_t column) noexcept
// 	{
// 		if (IsTreeItemSelected(pTreeItem.get()))
// 			return;
// 			
// 		pTreeItem->SetBackgroundColor(TreeDefaultColors::RowHoverColor);
// 	}
// 
// 	void OutlinerPanel::OnMouseExitTreeItemRow(std::shared_ptr<TreeItem> pTreeItem) noexcept
// 	{
// 		const bool isVisible = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem)->IsVisible();
// 		if (!isVisible && !IsTreeItemSelected(pTreeItem.get()))
// 			pTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
// 		
// 		if (!IsTreeItemSelected(pTreeItem.get()))
// 		{
// 			if (isVisible)
// 				pTreeItem->SetIconTint(0u, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
// 
// 			std::vector<std::shared_ptr<TreeItem>> descendants = m_pOutliner->GetDescendants(pTreeItem);
// 
// 			if (std::any_of(descendants.begin(), descendants.end(), [this](const std::shared_ptr<TreeItem>& currentTreeItem)
// 				{
// 					return IsTreeItemSelected(currentTreeItem.get());
// 				}))
// 			{
// 				pTreeItem->SetBackgroundColor(TreeDefaultColors::RowAncestorToSelectedColor);
// 			}
// 			else
// 				pTreeItem->ResetBackgroundColor();
// 		}
// 	}
// 
// 	void OutlinerPanel::OnMouseReleasedOnTreeItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept
// 	{
// 		if (!IsTreeItemSelectableAtClickedColumn(pTreeItem.get(), column))
// 			return;
// 
// 		if (!IsTreeItemSelected(pTreeItem.get()))
// 			return;
// 		
// 		if (GetNumSelected() <= 1)
// 			return;
// 		
// 		OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);
// 
// 		switch (OutlinerPanel_private::GetSelectionMode())
// 		{
// 		case OutlinerPanel_private::SelectionMode::Single:
// 		{
// 			DeselectAllTreeItems();
// 			SelectTreeItem(pOutlinerTreeItem);
// 			SetReferenceSelection(pOutlinerTreeItem);
// 			break;
// 		}
// 		}
// 	}
// 
// 	bool OutlinerPanel::OnDragOver(const Any& payload, const Any& target, std::string_view dragContext) noexcept
// 	{
// 		if (dragContext != "TreeItem")
// 			return false;
// 
// 		const std::shared_ptr<TreeItem> pPayload = *payload.Get<std::shared_ptr<TreeItem>>();
// 		const std::shared_ptr<TreeItem> pTarget = *target.Get<std::shared_ptr<TreeItem>>();
// 
// 		const std::vector<std::shared_ptr<TreeItem>> selectedTreeItems = GetAllSelectedTreeItems();
// 
// 		if (std::any_of(selectedTreeItems.begin(), selectedTreeItems.end(), [&](const std::shared_ptr<TreeItem>& pTreeItem)
// 			{
// 				return pTreeItem == pTarget;
// 			}))
// 		{
// 			m_DragDropTooltip = "Cannot attach entity to self";
// 			m_DragDropTooltipIcon = m_NotAllowedTextureIconHandle;
// 			return false;
// 		}
// 
// 		OutlinerTreeItem* pTargetOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTarget);
// 
// 		if (std::any_of(selectedTreeItems.begin(), selectedTreeItems.end(), [&](const std::shared_ptr<TreeItem>& pTreeItem)
// 			{
// 				OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);
// 				return pOutlinerTreeItem->GetType() == ETreeItemType::Filter && pTargetOutlinerTreeItem->GetType() == ETreeItemType::Entity;
// 			}))
// 		{
// 			m_DragDropTooltip = "Cannot attach filters to entities.";
// 			m_DragDropTooltipIcon = m_NotAllowedTextureIconHandle;
// 			return false;
// 		}
// 
// 		//Is pTarget the descendant of any of the selected? If so, return false
// 		//We ensure uniqueness with the unordered_set!
// 
// 		std::unordered_set<std::shared_ptr<TreeItem>> allDescendants;
// 		for (auto& selected : selectedTreeItems)
// 		{
// 			const std::vector<std::shared_ptr<TreeItem>> descendants = m_pOutliner->GetDescendants(selected);
// 			allDescendants.insert(descendants.begin(), descendants.end());
// 		}
// 
// 		if (std::any_of(allDescendants.begin(), allDescendants.end(), [&](const std::shared_ptr<TreeItem>& pDescendant)
// 			{
// 				return pDescendant == pTarget;
// 			}))
// 		{
// 			m_DragDropTooltip = "Parent cannot become the child of their descendant.";
// 			m_DragDropTooltipIcon = m_NotAllowedTextureIconHandle;
// 			return false;
// 		}
// 
// 		m_DragDropTooltip = pTarget->GetColumnLabel(1);
// 		m_DragDropTooltipIcon = m_CheckTextureIconHandle;
// 
// 		m_DraggedOnValidTargetThisFrame = true;
// 		return true;
// 	}
// 
// 	void OutlinerPanel::OnDrop([[maybe_unused]] const Any& payload, const Any& target, std::string_view dropContext) noexcept
// 	{
// 		if (dropContext != "TreeItem")
// 			return;
// 
// 		OutlinerTreeItem* pTargetTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(*target.Get<std::shared_ptr<TreeItem>>());
// 		const ETreeItemType targetType = pTargetTreeItem->GetType();
// 
// 		std::vector<OutlinerEntityTreeItem*> entityTreeItems;
// 		std::vector<OutlinerFilterTreeItem*> filterTreeItems;
// 
// 		const std::vector<std::shared_ptr<TreeItem>> allSelectedTreeItems = GetAllSelectedTreeItems();
// 		for (auto& selectedTreeItem : allSelectedTreeItems)
// 		{
// 			OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(selectedTreeItem);
// 			switch (pOutlinerTreeItem->GetType())
// 			{
// 			case ETreeItemType::Entity:
// 			{
// 				entityTreeItems.push_back(OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem));
// 				break;
// 			}
// 			case ETreeItemType::Filter:
// 			{
// 				filterTreeItems.push_back(OutlinerPanel_private::AsOutlinerFilterTreeItem(pOutlinerTreeItem));
// 				break;
// 			}
// 			default:
// 				break;
// 			}
// 		}
// 
// 		switch (targetType)
// 		{
// 		case ETreeItemType::Entity:
// 		{
// 			OutlinerEntityTreeItem* pTargetEntityTreeItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pTargetTreeItem);
// 			const entity targetEntity = pTargetEntityTreeItem->GetEntityID();
// 
// 			if (std::all_of(entityTreeItems.begin(), entityTreeItems.end(), [targetEntity, this](OutlinerEntityTreeItem* pEntityTreeItem)
// 				{
// 					const entity currentEntity = pEntityTreeItem->GetEntityID();
// 					return m_pScene->EntityIsParent(currentEntity, targetEntity);
// 				}))
// 			{
// 				TreeItem* pGrandParentTreeItem = pTargetEntityTreeItem->GetParent();
// 
// 				for (OutlinerEntityTreeItem* pEntityTreeItem : entityTreeItems)
// 					m_pScene->DetachEntity(pEntityTreeItem->GetEntityID());
// 
// 				if (pGrandParentTreeItem)
// 				{
// 					OutlinerTreeItem* pGrandParentOutlinerTreeItem = static_cast<OutlinerTreeItem*>(pGrandParentTreeItem);
// 					switch (pGrandParentOutlinerTreeItem->GetType())
// 					{
// 					case ETreeItemType::Entity:
// 					{
// 						OutlinerEntityTreeItem* pGrandParentEntityTreeItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pGrandParentOutlinerTreeItem);
// 
// 						for (OutlinerEntityTreeItem* pEntityTreeItem : entityTreeItems)
// 							m_pScene->AttachEntity(pEntityTreeItem->GetEntityID(), pGrandParentEntityTreeItem->GetEntityID());
// 						
// 						break;
// 					}
// 					case ETreeItemType::Filter:
// 					{
// 						OutlinerFilterTreeItem* pGrandParentFilterTreeItem = OutlinerPanel_private::AsOutlinerFilterTreeItem(pGrandParentOutlinerTreeItem);
// 						for (OutlinerEntityTreeItem* pEntityTreeItem : entityTreeItems)
// 							m_pEditor->GetEntityFiltersManager()->SetEntityToFilter(pEntityTreeItem->GetEntityID(), pGrandParentFilterTreeItem->GetPath());
// 
// 						break;
// 					}
// 					}
// 				}
// 			}
// 			else
// 			{
// 				for (OutlinerEntityTreeItem* pEntityTreeItem : entityTreeItems)
// 				{
// 					m_pScene->DetachEntity(pEntityTreeItem->GetEntityID());
// 				}
// 
// 				for (OutlinerEntityTreeItem* pEntityTreeItem : entityTreeItems)
// 				{
// 					m_pScene->AttachEntity(pEntityTreeItem->GetEntityID(), targetEntity);
// 				}
// 			}
// 			break;
// 		}
// 		case ETreeItemType::Filter:
// 		{
// 			entityTreeItems.erase(std::remove_if(entityTreeItems.begin(), entityTreeItems.end(),
// 				[&](OutlinerEntityTreeItem* pCurrentEntityTreeItem)
// 				{
// 					return std::any_of(entityTreeItems.begin(), entityTreeItems.end(), [&](OutlinerEntityTreeItem* pPossibleAncestor)
// 						{
// 							return m_pScene->EntityIsAncestor(pPossibleAncestor->GetEntityID(), pCurrentEntityTreeItem->GetEntityID());
// 						});
// 				}),
// 				entityTreeItems.end());
// 
// 			EntityFiltersManager& filterManager = *m_pEditor->GetEntityFiltersManager();
// 
// 			OutlinerFilterTreeItem* pTargetFilterTreeItem = OutlinerPanel_private::AsOutlinerFilterTreeItem(pTargetTreeItem);
// 			const std::string& targetFilterPath = pTargetFilterTreeItem->GetPath();
// 
// 			for (OutlinerEntityTreeItem* pEntityTreeItem : entityTreeItems)
// 				filterManager.SetEntityToFilter(pEntityTreeItem->GetEntityID(), targetFilterPath);
// 
// 			for (OutlinerFilterTreeItem* pFilterTreeItem : filterTreeItems)
// 				filterManager.SetFilterToFilter(pFilterTreeItem->GetPath(), targetFilterPath);
// 
// 			break;
// 		}
// 		}
// 	}
// 
// 	void OutlinerPanel::OnEntityFilterCreated(const std::string& path) noexcept
// 	{
// 		std::shared_ptr<EntityFilter> pFilter = m_pEditor->GetEntityFiltersManager()->GetFilter(path);
// 		std::vector<EntityFilter*> hierarchy;
// 
// 		EntityFilter* pCurrent = pFilter.get();
// 		while (pCurrent)
// 		{
// 			hierarchy.push_back(pCurrent);
// 			pCurrent = pCurrent->GetParent();
// 		}
// 
// 		//Hierarchy is stored in reverse, so traverse it so:
// 		const int lastIndex = static_cast<int>(hierarchy.size()) - 1;
// 
// 		for (int i = lastIndex; i >= 0; --i)
// 		{
// 			EntityFilter* pCurrentFilter = hierarchy[i];
// 			if (m_FilterPathToTreeItemMap.contains(pCurrentFilter->GetPath()))
// 				continue;
// 
// 			//The current filter is now eligible for being created, and its parent, if any, should be the previously processed filter.
// 			std::shared_ptr<OutlinerFilterTreeItem> pFilterTreeItem = CreateFilterTreeItem(pCurrentFilter);
// 
// 			if (i != lastIndex)
// 				m_FilterPathToTreeItemMap[hierarchy[i + 1]->GetPath()]->AddChild(pFilterTreeItem);
// 			else
// 				AddToRoot(pFilterTreeItem);
// 		}
// 	}
// 
// 	void OutlinerPanel::OnEntityFilterDestroyed(const std::string& path) noexcept
// 	{
// 		std::shared_ptr<OutlinerFilterTreeItem> pFilter = m_FilterPathToTreeItemMap[path];
// 
// 		if (m_SelectedEntityFilters.contains(pFilter.get()))
// 			m_SelectedEntityFilters.erase(pFilter.get());
// 
// 		if (pFilter->HasParent())
// 			pFilter->GetParent()->RemoveChild(pFilter);
// 
// 		m_FilterPathToTreeItemMap.erase(path);
// 	}
// 
// 	void OutlinerPanel::OnEntitySetToFilter(entity e, const std::string& filterPath) noexcept
// 	{
// 		m_SuspendNotifications = true;
// 
// 		const std::shared_ptr<OutlinerTreeItem>& pEntityTreeItem = m_EntityToTreeItemMap[e];
// 		const std::shared_ptr<OutlinerFilterTreeItem>& pFilterTreeItem = m_FilterPathToTreeItemMap[filterPath];
// 
// 		if (pEntityTreeItem->HasParent())
// 			pEntityTreeItem->GetParent()->RemoveChild(pEntityTreeItem);
// 
// 		if (m_pScene->HasParent(e))
// 			m_pScene->DetachEntity(e);
// 
// 		pFilterTreeItem->AddChild(pEntityTreeItem);
// 
// 		m_SuspendNotifications = false;
// 	}
// 
// 	void OutlinerPanel::OnEntityRemovedFromFilter(entity e, const std::string& filterPath, bool filterToBeDestroyed) noexcept
// 	{
// 		const std::shared_ptr<OutlinerTreeItem>& pEntityTreeItem = m_EntityToTreeItemMap[e];
// 		const std::shared_ptr<OutlinerFilterTreeItem>& pFilterTreeItem = m_FilterPathToTreeItemMap[filterPath];
// 
// 		pFilterTreeItem->RemoveChild(pEntityTreeItem);
// 
// 		if (filterToBeDestroyed && m_pEditor->GetEntityFiltersManager()->IsRootFilter(filterPath))
// 		{
// 			//We need to manually set the entity to the scene tree item in this case.
// 			m_pOutliner->GetRootEntries().front()->AddChild(pEntityTreeItem);
// 		}
// 	}
// 
// 	void OutlinerPanel::OnEntityFilterReattached(const std::string& childFilterPathOld, const std::string& childFilterPathNew, const std::string& parentFilterPath) noexcept
// 	{
// 		const std::string currentChildFilterPath = childFilterPathOld;
// 
// 		std::shared_ptr<OutlinerFilterTreeItem> pChildFilter = m_FilterPathToTreeItemMap[currentChildFilterPath];
// 
// 		if (pChildFilter->HasParent())
// 			pChildFilter->GetParent()->RemoveChild(pChildFilter);
// 
// 		pChildFilter->SetPath(childFilterPathNew);
// 		
// 		std::function<void(OutlinerFilterTreeItem* pCurrentFilter, const std::string& path)> PropagatePathChanges 
// 			= [&](OutlinerFilterTreeItem* pCurrentFilter, const std::string& path)
// 		{
// 			const std::string fullPath = path.empty() ? pCurrentFilter->GetPath() : path + "/" + pCurrentFilter->GetColumnLabel(1);
// 
// 			if (!path.empty())
// 			{
// 				m_FilterPathToTreeItemMap[fullPath] = m_FilterPathToTreeItemMap[pCurrentFilter->GetPath()];
// 				m_FilterPathToTreeItemMap.erase(pCurrentFilter->GetPath());
// 				
// 				pCurrentFilter->SetPath(fullPath);
// 			}
// 			
// 			const std::vector<std::shared_ptr<TreeItem>>& children = pCurrentFilter->GetChildren();
// 			for (auto& child : children)
// 			{
// 				OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(child);
// 				const ETreeItemType type = pOutlinerTreeItem->GetType();
// 
// 				if (type != ETreeItemType::Filter)
// 					continue;
// 
// 				OutlinerFilterTreeItem* pFilterTreeItem = OutlinerPanel_private::AsOutlinerFilterTreeItem(pOutlinerTreeItem);
// 				PropagatePathChanges(pFilterTreeItem, fullPath);
// 			}
// 		};
// 
// 		PropagatePathChanges(pChildFilter.get(), "");
// 
// 		if (parentFilterPath.empty())
// 			m_pOutliner->GetRootEntries().front()->AddChild(pChildFilter);
// 		else
// 		{
// 			const std::shared_ptr<OutlinerFilterTreeItem>& pParentFilter = m_FilterPathToTreeItemMap[parentFilterPath];
// 			pParentFilter->AddChild(pChildFilter);
// 		}
// 
// 		m_FilterPathToTreeItemMap.erase(currentChildFilterPath);
// 		m_FilterPathToTreeItemMap[childFilterPathNew] = pChildFilter;
// 	}
// 
// 	void OutlinerPanel::SetAndPropagateTreeItemVisibility(OutlinerTreeItem* pOutlinerTreeItem, bool visibilityState) noexcept
// 	{
// 		const bool isSelected = IsTreeItemSelected(pOutlinerTreeItem);
// 
// 		pOutlinerTreeItem->SetVisibility(visibilityState);
// 		pOutlinerTreeItem->SetVisibilityIcon(visibilityState ? m_ShowEntityTextureIconHandle : m_HideEntityTextureIconHandle);
// 		
// 		if (pOutlinerTreeItem->GetType() == ETreeItemType::Entity)
// 		{
// 			const entity e = OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem)->GetEntityID();
// 			EntityManager& entityManager = m_pScene->GetEntityManager();
// 			
// 			if (visibilityState == true)
// 			{
// 				if (entityManager.Has<HiddenInGameComponent>(e))
// 					entityManager.Remove<HiddenInGameComponent>(e);
// 			}
// 			else
// 				entityManager.AddOrReplace<HiddenInGameComponent>(e);
// 		}
// 
// 		const std::vector<std::shared_ptr<TreeItem>>& children = pOutlinerTreeItem->GetChildren();
// 		
// 		if (isSelected)
// 			pOutlinerTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
// 		else
// 		{
// 			if (visibilityState == false)
// 				pOutlinerTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
// 			else
// 				pOutlinerTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
// 		}
// 
// 		for (auto& child : children)
// 			SetAndPropagateTreeItemVisibility(static_cast<OutlinerTreeItem*>(child.get()), visibilityState);
// 	}
// 
// 	void OutlinerPanel::SetReferenceSelection(OutlinerTreeItem* pReferenceSelection)
// 	{
// 		m_pReferenceSelection = pReferenceSelection;
// 	}
// 
// 	void OutlinerPanel::DetermineAndIssueSelection(OutlinerTreeItem* pTreeItem, uint32_t clickedColumn) noexcept
// 	{
// 		using namespace OutlinerPanel_private;
// 
// 		const bool isSelected = IsTreeItemSelected(pTreeItem);
// 
// 		switch (GetSelectionMode())
// 		{
// 		case SelectionMode::Single:
// 		{
// 			if (GetNumSelected() > 1 && isSelected)
// 				return;
// 
// 			if (IsTreeItemSelectableAtClickedColumn(pTreeItem, clickedColumn))
// 			{
// 				DeselectAllTreeItems();
// 				SelectTreeItem(pTreeItem);
// 				SetReferenceSelection(pTreeItem);
// 			}
// 
// 			break;
// 		}
// 		case SelectionMode::Toggle:
// 		{
// 			if (IsTreeItemSelected(pTreeItem))
// 			{
// 				DeselectTreeItem(pTreeItem);
// 				if (GetNumSelected() > 0)
// 				{
// 					switch (m_LastSelectedType)
// 					{
// 					case ETreeItemType::Entity:
// 					{
// 						const entity firstSelected = m_pEditor->GetSelection()->GetFirstSelected();
// 						RLS_ASSERT(firstSelected != NULL_ENTITY, "[OutlinerPanel]: Invalid enitity ID!");
// 						SetReferenceSelection(m_EntityToTreeItemMap[firstSelected].get());
// 						break;
// 					}
// 					default:
// 						break;
// 					}
// 				}
// 			}
// 			else
// 			{
// 				if (IsTreeItemSelectableAtClickedColumn(pTreeItem, clickedColumn))
// 				{
// 					SelectTreeItem(pTreeItem);
// 					SetReferenceSelection(pTreeItem);
// 				}
// 			}
// 			break;
// 		}
// 		case SelectionMode::Range:
// 		{
// 			const std::vector<Tree::TreeDataRow> treeDatas = m_pOutliner->FlattenTree();
// 			if (treeDatas.empty())
// 				break;
// 
// 			size_t startIndex = 0u;
// 			if (m_pReferenceSelection)
// 			{
// 				auto it = std::find_if(treeDatas.begin(), treeDatas.end(), [&](const Tree::TreeDataRow& row)
// 					{
// 						return row.Entry.get() == m_pReferenceSelection;
// 					});
// 				startIndex = std::distance(treeDatas.begin(), it);
// 			}
// 			else
// 				m_pReferenceSelection = treeDatas[0].Entry.get();
// 
// 			auto it = std::find_if(treeDatas.begin(), treeDatas.end(), [&](const Tree::TreeDataRow& row)
// 				{
// 					return row.Entry.get() == pTreeItem;
// 				});
// 
// 			size_t endIndex = std::distance(treeDatas.begin(), it);
// 
// 			if (startIndex > endIndex)
// 				std::swap(startIndex, endIndex);
// 
// 			for (size_t i = startIndex; i <= endIndex; ++i)
// 			{
// 				if (IsTreeItemSelectableAtClickedColumn(treeDatas[i].Entry.get(), clickedColumn) && !IsTreeItemSelected(treeDatas[i].Entry.get()))
// 					SelectTreeItem(static_cast<OutlinerTreeItem*>(treeDatas[i].Entry.get()));
// 			}
// 
// 			break;
// 		}
// 		}
// 	}
// 
// 	void OutlinerPanel::SelectTreeItem(OutlinerTreeItem* pTreeItem) noexcept
// 	{
// 		switch (pTreeItem->GetType())
// 		{
// 		case ETreeItemType::Entity:
// 		{
// 			m_pEditor->GetSelection()->SelectEntity(OutlinerPanel_private::AsOutlinerEntityTreeItem(pTreeItem)->GetEntityID());
// 			break;
// 		}
// 		case ETreeItemType::Scene:
// 		{
// 			m_SceneTreeItemSelected = true;
// 			pTreeItem->SetBackgroundColor(m_pOutliner->IsFocused() ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
// 			pTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
// 			break;
// 		}
// 		case ETreeItemType::Filter:
// 		{
// 			m_SelectedEntityFilters.insert(pTreeItem);
// 			pTreeItem->SetBackgroundColor(m_pOutliner->IsFocused() ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
// 			pTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
// 			break;
// 		}
// 		}
// 
// 		std::vector<TreeItem*> ancestors = pTreeItem->GetAncestors();
// 		for (TreeItem* ancestor : ancestors)
// 		{
// 			if (!IsTreeItemSelected(ancestor))
// 				ancestor->SetBackgroundColor(TreeDefaultColors::RowAncestorToSelectedColor);
// 		}
// 	}
// 
// 	void OutlinerPanel::DeselectTreeItem(OutlinerTreeItem* pTreeItem) noexcept
// 	{
// 		switch (pTreeItem->GetType())
// 		{
// 		case ETreeItemType::Entity:
// 		{
// 			OutlinerEntityTreeItem* pEntityTreeItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pTreeItem);
// 			m_pEditor->GetSelection()->DeselectEntity(pEntityTreeItem->GetEntityID());
// 			break;
// 		}
// 		case ETreeItemType::Scene:
// 		{
// 			m_SceneTreeItemSelected = false;
// 			pTreeItem->ResetBackgroundColor();
// 			const ImVec4 iconTint = pTreeItem->IsVisible() ? ImVec4(1.0f, 1.0f, 1.0f, 0.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
// 			pTreeItem->SetIconTint(0u, iconTint);
// 			break;
// 		}
// 		case ETreeItemType::Filter:
// 		{
// 			m_SelectedEntityFilters.erase(pTreeItem);
// 			pTreeItem->ResetBackgroundColor();
// 			const ImVec4 iconTint = pTreeItem->IsVisible() ? ImVec4(1.0f, 1.0f, 1.0f, 0.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
// 			pTreeItem->SetIconTint(0u, iconTint);
// 
// 			std::vector<TreeItem*> ancestors = pTreeItem->GetAncestors();
// 			for (TreeItem* ancestor : ancestors)
// 			{
// 				std::vector<TreeItem*> ancestorsDescendants = ancestor->GetDescendants();
// 				const bool noneIsSelected = std::all_of(ancestorsDescendants.begin(), ancestorsDescendants.end(), [this](TreeItem* pCurrentTreeItem)
// 					{
// 						return IsTreeItemSelected(pCurrentTreeItem) == false;
// 					});
// 
// 				if (noneIsSelected)
// 					ancestor->ResetBackgroundColor();
// 			}
// 
// 			break;
// 		}
// 		}
// 	}
// 
// 	void OutlinerPanel::DeselectAllTreeItems() noexcept
// 	{
// 		m_pEditor->GetSelection()->DeselectAllEntities();
// 
// 		if (m_SceneTreeItemSelected)
// 			DeselectTreeItem(OutlinerPanel_private::AsOutlinerTreeItem(m_pOutliner->GetRootEntries().front()));
// 
// 		std::vector<OutlinerTreeItem*> selectedFilters;
// 		selectedFilters.reserve(m_SelectedEntityFilters.size());
// 
// 		for (auto filter : m_SelectedEntityFilters)
// 			selectedFilters.push_back(filter);
// 
// 		for (auto filter : selectedFilters)
// 			DeselectTreeItem(filter);
// 	}
// 
// 	bool OutlinerPanel::IsTreeItemSelected(TreeItem* pTreeItem) const noexcept
// 	{
// 		OutlinerTreeItem* pOutlinerTreeItem = static_cast<OutlinerTreeItem*>(pTreeItem);
// 
// 		switch (pOutlinerTreeItem->GetType())
// 		{
// 		case ETreeItemType::Entity: 
// 		{
// 			OutlinerEntityTreeItem* pEntityItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem);
// 			return m_pEditor->GetSelection()->IsEntitySelected(pEntityItem->GetEntityID());
// 		}
// 		case ETreeItemType::Scene:
// 		{
// 			return m_SceneTreeItemSelected;
// 		}
// 		case ETreeItemType::Filter:
// 		{
// 			return m_SelectedEntityFilters.contains(pOutlinerTreeItem);
// 		}
// 		default:
// 		{
// 			RLS_ASSERT(false, "Unreachable.");
// 			return false;
// 		}
// 		}
// 	}
// 
// 	bool OutlinerPanel::IsTreeItemSelectableAtClickedColumn([[maybe_unused]] TreeItem* pTreeItem, uint32_t column) const noexcept
// 	{
// 		if (column == 0)
// 			return false;
// 
// 		return true;
// 	}
// 
// 	std::vector<std::shared_ptr<TreeItem>> OutlinerPanel::GetAllSelectedTreeItems() const noexcept
// 	{
// 		std::vector<std::shared_ptr<TreeItem>> selectedTreeItems;
// 		selectedTreeItems.reserve(GetNumSelected());
// 
// 		const std::vector<entity>& selectedEntities = m_pEditor->GetSelection()->GetSelectedEntities();
// 
// 		for (const entity selectedEntity : selectedEntities)
// 			selectedTreeItems.push_back(m_EntityToTreeItemMap.at(selectedEntity));
// 
// 		if (m_SceneTreeItemSelected)
// 			selectedTreeItems.push_back(m_pOutliner->GetRootEntries().front());
// 
// 		for (auto filter : m_SelectedEntityFilters)
// 		{
// 			const std::string& filterPath = OutlinerPanel_private::AsOutlinerFilterTreeItem(filter)->GetPath();
// 			selectedTreeItems.push_back(m_FilterPathToTreeItemMap[filterPath]);
// 		}
// 
// 		return selectedTreeItems;
// 	}
// 
// 	uint32_t OutlinerPanel::GetNumSelected() const noexcept
// 	{
// 		uint32 selectedTreeItemCount = 0u;
// 		
// 		selectedTreeItemCount += m_pEditor->GetSelection()->GetSelectedEntityCount();
// 
// 		if (m_SceneTreeItemSelected)
// 			selectedTreeItemCount++;
// 
// 		selectedTreeItemCount += static_cast<uint32>(m_SelectedEntityFilters.size());
// 
// 		return selectedTreeItemCount;
// 	}
// 
// 	OutlinerSceneTreeItem* OutlinerPanel::GetSceneTreeItem() const noexcept
// 	{
// 		return static_cast<OutlinerSceneTreeItem*>(m_pOutliner->GetRootEntries().front().get());
// 	}
// 
// 	OutlinerTreeItem* OutlinerPanel::GetFirstNonEntityTreeItemAncestor(TreeItem* pTreeItem) const noexcept
// 	{
// 		RLS_ASSERT(pTreeItem, "[OutlinerPanel]: Tree item is invalid!");
// 
// 		TreeItem* pCurrentTreeItem = pTreeItem->GetParent();
// 		while (static_cast<OutlinerTreeItem*>(pCurrentTreeItem)->GetType() == ETreeItemType::Entity)
// 		{
// 			pCurrentTreeItem = pCurrentTreeItem->GetParent();
// 		}
// 
// 		return static_cast<OutlinerTreeItem*>(pCurrentTreeItem);
// 	}
// 
// }