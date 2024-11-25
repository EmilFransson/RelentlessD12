#include "OutlinerPanel.h"
#include "../Core/Editor.h"

namespace Relentless
{
	namespace OutlinerPanel_private
	{
		[[nodiscard]] OutlinerTreeItem* AsOutlinerTreeItem(const std::shared_ptr<TreeItem>& pTreeItem)
		{
			return static_cast<OutlinerTreeItem*>(pTreeItem.get());
		}

		[[nodiscard]] OutlinerEntityTreeItem* AsOutlinerEntityTreeItem(OutlinerTreeItem* pOutlinerTreeItem)
		{
			return static_cast<OutlinerEntityTreeItem*>(pOutlinerTreeItem);
		}

		[[nodiscard]] OutlinerSceneTreeItem* AsOutlinerSceneTreeItem(OutlinerTreeItem* pOutlinerTreeItem)
		{
			return static_cast<OutlinerSceneTreeItem*>(pOutlinerTreeItem);
		}

		[[nodiscard]] OutlinerFolderTreeItem* AsOutlinerTreeItem(OutlinerTreeItem* pOutlinerTreeItem)
		{
			return static_cast<OutlinerFolderTreeItem*>(pOutlinerTreeItem);
		}

		enum class SelectionMode : uint8_t { Single, Toggle, Range };

		[[nodiscard]] SelectionMode GetSelectionMode() noexcept
		{
			if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
				return SelectionMode::Toggle;
			else if (Keyboard::IsKeyPressed(RLS_KEY::LShift))
				return SelectionMode::Range;
			else
				return SelectionMode::Single;
		}
	}

	OutlinerPanel::OutlinerPanel(Editor* pEditor) noexcept
		: m_pEditor{ pEditor }
	{
		pEditor->OnSceneChanged.Connect(this, &OutlinerPanel::OnEditorSceneChanged);
		pEditor->GetSelection().OnSelectionChanged.Connect(this, &OutlinerPanel::OnEntitySelectionChangedFromEditor);
		
		m_pTreeInteraction = std::make_shared<TreeInteraction>();
		m_pTreeInteraction->OnItemClicked.Connect(this, &OutlinerPanel::OnTreeItemClicked);
		m_pTreeInteraction->OnItemHovered.Connect(this, &OutlinerPanel::OnTreeItemHovered);
		m_pTreeInteraction->OnMouseEnterItemRow.Connect(this, &OutlinerPanel::OnMouseEnterTreeItemRow);
		m_pTreeInteraction->OnMouseExitItemRow.Connect(this, &OutlinerPanel::OnMouseExitTreeItemRow);
		m_pTreeInteraction->OnMouseReleasedOnItem.Connect(this, &OutlinerPanel::OnMouseReleasedOnTreeItem);

		std::shared_ptr<TreeStyle> pTreeStyle = std::make_shared<TreeStyle>();
		pTreeStyle->SetUseAlternatingRowColors(true);

		std::shared_ptr<TreeDataView> pDataView = std::make_shared<TreeDataView>();
		
		m_pDragDropBehavior = std::make_shared<DragDropBehavior>("OutlinerTreeDragDrop");
		m_pDragDropBehavior->OnDragOver.Connect(this, &OutlinerPanel::OnDragOver);
		m_pDragDropBehavior->OnDrop.Connect(this, &OutlinerPanel::OnDrop);

		pDataView->pTreeInteraction = m_pTreeInteraction;
		pDataView->pTreeStyle = pTreeStyle;
		pDataView->pDragDropBehavior = m_pDragDropBehavior;

		m_pOutliner = std::make_shared<Outliner>(pDataView);
		m_pOutliner->OnFocusChanged.Connect(this, &OutlinerPanel::OnOutlinerTreeFocusChanged);

		RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\showicon.rasset"), m_ShowEntityTextureIconHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures\\Icons\\hideicon.rasset"), m_HideEntityTextureIconHandle), "Core engine icon missing.");
	}

	OutlinerPanel::~OutlinerPanel() noexcept
	{
		if (m_pEditor)
		{
			m_pEditor->OnSceneChanged.Detach(this);
			m_pEditor->GetSelection().OnSelectionChanged.Detach(this);
		}

		if (m_pScene)
		{
			m_pScene->OnEntityCreated.Detach(this);
			m_pScene->OnEntityDestroyed.Detach(this);
			m_pScene->OnEntityAttached.Detach(this);
			m_pScene->OnEntityDetached.Detach(this);
			m_pScene->OnEntityVisibilityChanged.Detach(this);
		}

		if (m_pOutliner)
			m_pOutliner->OnFocusChanged.Detach(this);

		if (m_pTreeInteraction)
		{
			m_pTreeInteraction->OnItemClicked.Detach(this);
			m_pTreeInteraction->OnItemHovered.Detach(this);
			m_pTreeInteraction->OnMouseEnterItemRow.Detach(this);
			m_pTreeInteraction->OnMouseExitItemRow.Detach(this);
			m_pTreeInteraction->OnMouseReleasedOnItem.Detach(this);
		}

		if (m_pDragDropBehavior)
		{
			m_pDragDropBehavior->OnDragOver.Detach(this);
			m_pDragDropBehavior->OnDrop.Detach(this);
		}
	}

	void OutlinerPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;
		
		PROFILE_FUNC;
		
		m_DraggedOnValidTargetThisFrame = false;
		m_DragDropTooltip.clear();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Outliner");
		ImGui::PopStyleVar();
		
		constexpr float bannerHeight = 40.0f;

		{
			ImGui::BeginChild("Outliner SearchBar Child", ImVec2(0.0f, 50.0f));
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			UI::SearchBar("OutlinerSearchBar", "Search...", true);
			ImGui::EndChild();
		}

		{
			ImGui::BeginChild("Outliner Table Child", ImVec2(0, ImGui::GetContentRegionAvail().y - bannerHeight));
			m_pOutliner->Draw();

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !m_pOutliner->IsHovered())
				m_pEditor->GetSelection().DeselectAllEntities();

			ImGui::EndChild();
		}

		{
			ImGui::BeginChild("Outliner Banner Child");
			
			const ImVec2 bannerStartPos = ImGui::GetCursorScreenPos();
			const ImVec2 availableSpace = ImGui::GetContentRegionAvail();
			const ImVec2 bannerEndPos = ImVec2(bannerStartPos.x + availableSpace.x, bannerStartPos.y + availableSpace.y);

			ImDrawList* pDrawList = ImGui::GetWindowDrawList();
			pDrawList->AddRectFilled(bannerStartPos, bannerEndPos, ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)));

			const uint32_t numEntities = m_pScene->GetEntityManager().GetEntityAliveCount();
			const uint32_t selectedEntities = m_pEditor->GetSelection().GetSelectedEntityCount();

			std::string bannerLabel = std::format("{} Entities", numEntities);
			if (selectedEntities > 0)
				bannerLabel += std::format(" ({} selected)", selectedEntities);

			constexpr float bannerLabelOffsetFromBorder = 10.0f;
			const float bannerLabelXPos = bannerStartPos.x + bannerLabelOffsetFromBorder;

			const float bannerLabelHeight = UI::Utility::CalculateTextHeight(bannerLabel);
			const float bannerLabelYPos = bannerStartPos.y + (bannerHeight / 2.0f) - (bannerLabelHeight / 2.0f);

			pDrawList->AddText(ImVec2(bannerLabelXPos, bannerLabelYPos), ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), bannerLabel.c_str());

			ImGui::EndChild();
		}

		ImGui::End();

		if (m_pDragDropBehavior->IsActive())
		{
			if (!m_DragDropTooltip.empty())
				ImGui::SetTooltip(m_DragDropTooltip.c_str());
		}

		return;

		//m_HoveredEntity = NULL_ENTITY;
		//
		//EntityManager& entityManager = m_pScene->GetEntityManager();
		//
		//for (uint32_t i = 0u; i < m_SelectedEntities.size(); ++i)
		//{
		//	if (!entityManager.Exists(m_SelectedEntities[i]))
		//		m_SelectedEntities.erase(m_SelectedEntities.begin() + i);
		//}
		//
		//m_SceneIsHiddenInGame = entityManager.GetEntityCountForPool<HiddenInGameComponent>() == entityManager.GetEntityAliveCount();
		//
		//
		//constexpr ImVec4 evenTableRowBgColor = ImVec4(21.0f / 255.0f, 21.0f / 255.0f, 21.0f / 255.0f, 255.0f / 255.0f);
		//constexpr ImVec4 oddTableRowBgColor = ImVec4(26.0f / 255.0f, 26.0f / 255.0f, 26.0f / 255.0f, 255.0f / 255.0f);
		//
		//ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBg, evenTableRowBgColor);
		//ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBgAlt, oddTableRowBgColor);
		//
		//constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
		//
		//bool hoversAnyRow = false;
		//
		//if (ImGui::BeginTable("OutlinerTable", SceneHierarchyPanel_private::TABLE_COLUMN_COUNT, flags))
		//{
		//	m_IsTableFocused = ImGui::IsWindowFocused();
		//
		//	ImGui::TableSetupScrollFreeze(0, 1);
		//
		//	ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 30.0f);
		//	ImGui::TableSetupColumn("Item Label", ImGuiTableColumnFlags_WidthStretch);
		//	ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
		//	ImGui::TableHeadersRow();
		//
		//	for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
		//	{
		//		ImGui::TableSetColumnIndex(column);
		//		
		//		const ImRect headerRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
		//		if (ImGui::IsMouseHoveringRect(headerRect.Min, headerRect.Max))
		//		{
		//			const char* tooltip = (column == 0) ? "Visibility" : (column == 1) ? "Item Label" : "Displays the name of each entity's type";
		//			UI::Utility::DrawTooltip(tooltip);
		//		}
		//
		//		if (column == 0)
		//		{
		//			constexpr ImVec2 imageSize = ImVec2(22, 12);
		//			constexpr float imagePositionPadding = 3.0f;
		//			constexpr ImVec4 imageTint = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
		//
		//			const float availableWidth = ImGui::GetContentRegionAvail().x;
		//			const float availableHeight = ImGui::GetTextLineHeightWithSpacing();
		//			const float offsetX = (availableWidth - imageSize.x) * 0.5f;
		//			const float offsetY = (availableHeight - imageSize.y) * 0.5f;
		//
		//			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX + imagePositionPadding);
		//			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY - imagePositionPadding);
		//			
		//			const std::shared_ptr<Texture2D> pShowIconTexture = AssetManager::Get<Texture2D>(m_ShowEntityTextureIconHandle);
		//			ImGui::Image((ImTextureID)pShowIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, imageSize, ImVec2(0,0), ImVec2(1,1), imageTint);
		//		}
		//		else
		//			ImGui::TableHeader(ImGui::TableGetColumnName(column));
		//	}
		//
		//	bool isOpen = false;
		//	hoversAnyRow |= DrawTableRootEntry(isOpen);
		//
		//	if (isOpen)
		//	{
		//		m_pScene->GetEntityManager().Collect<NameComponent>().Do([&](entity currentEntity, const NameComponent& nameComponent)
		//			{
		//				if (!m_pScene->GetEntityManager().Has<RootComponent>(currentEntity))
		//					return;
		//
		//				const bool IsUsingFilter = !m_ContentFilter.empty();
		//				if (IsUsingFilter)
		//				{
		//					std::vector<entity> entititesToCheck = m_pScene->GetAllEntityDescendants(currentEntity);
		//					entititesToCheck.push_back(currentEntity);
		//
		//					EntityManager& entityManager = m_pScene->GetEntityManager();
		//					std::string contentFilterToLower = m_ContentFilter;
		//					std::transform(contentFilterToLower.begin(), contentFilterToLower.end(), contentFilterToLower.begin(), ::tolower);
		//
		//					if (std::all_of(entititesToCheck.begin(), entititesToCheck.end(), [this, &entityManager, &contentFilterToLower](entity entityToCheck)
		//						{
		//							auto& nc = entityManager.Get<NameComponent>(entityToCheck);
		//							std::string entryStemToLower = nc.Name;
		//							std::transform(entryStemToLower.begin(), entryStemToLower.end(), entryStemToLower.begin(), ::tolower);
		//
		//							return entryStemToLower.find(contentFilterToLower) == std::string::npos;
		//						}))
		//					{
		//						return;
		//					}
		//				}
		//
		//				hoversAnyRow |= DrawTableEntry(currentEntity);
		//			});
		//		ImGui::TreePop();
		//	}
		//	ImGui::EndTable();
		//}
		//
		//if (!hoversAnyRow)
		//	m_HoveredEntity = NULL_ENTITY;
		//
		//DrawDraggingTooltip();
		//
		//constexpr float padding = 7.0f;
		//const float currentYPosition = ImGui::GetCursorPosY();
		//const ImVec2 windowSize = ImGui::GetWindowSize();
		//const float remainingWindowHeight = windowSize.y - currentYPosition;
		//
		//ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), currentYPosition - padding));
		//
		//if (remainingWindowHeight > 0.0f)
		//{
		//	if (ImGui::InvisibleButton("OUTLINER_EMPTY_DROP_AREA", ImVec2(ImGui::GetContentRegionAvail().x, remainingWindowHeight)))
		//	{
		//		m_SelectedEntities.clear();
		//		m_SceneTableEntrySelected = false;
		//	}
		//
		//	m_TableEmptySpaceHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
		//
		//	if (ImGui::BeginDragDropTarget())
		//	{
		//		if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("OUTLINER_TABLE_PAYLOAD"))
		//		{
		//			const entity entityToDetach = *(entity*)payLoad->Data;
		//	
		//			EntityManager& entityManager = m_pScene->GetEntityManager();
		//	
		//			if (!std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager](entity e)
		//				{
		//					return entityManager.Has<RootComponent>(e);
		//				}))
		//			{
		//				std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e1)
		//					{
		//						if (!std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, e1](entity e2)
		//							{
		//								return m_pScene->EntityIsParent(e1, e2);
		//							}))
		//						{
		//							m_pScene->DetachEntity(e1);
		//						}
		//					});
		//			}
		//			ImGui::EndDragDropTarget();
		//		}
		//	}
		//}
		//
		//ImGui::PopStyleColor(2);
		//ImGui::End();

		////Any deletion of an entity in the scene hierarchy has been deferred until now:
		//if (m_EntityScheduledForDestruction != NULL_ENTITY)
		//{
		//	//We need to check if a child of the deleted entity is currently selected, and if so, notify the editor layer,
		//	//as that child will get destroyed in the process.
		//	if (m_pScene->GetEntityManager().Has<ParentComponent>(m_EntityScheduledForDestruction))
		//	{
		//		auto& children = m_pScene->GetEntityManager().Get<ParentComponent>(m_EntityScheduledForDestruction).Children;
		//		for (auto child : children)
		//		{
		//			if (child == m_SelectedEntity)
		//			{
		//				m_OnEntityDestroyedCallBack(child);
		//				m_SelectedEntity = NULL_ENTITY;
		//				break;
		//			}
		//		}
		//	}
		//
		//	m_pScene->DestroyEntity(m_EntityScheduledForDestruction);
		//	m_OnEntityDestroyedCallBack(m_EntityScheduledForDestruction);
		//	if (m_SelectedEntity == m_EntityScheduledForDestruction)
		//	{
		//		m_SelectedEntity = NULL_ENTITY;
		//	}
		//	m_EntityScheduledForDestruction = NULL_ENTITY;
		//}
	}

	void OutlinerPanel::OnEditorSceneChanged(Scene* pScene) noexcept
	{
		if (m_pScene)
		{
			m_pScene->OnEntityCreated.Detach(this);
			m_pScene->OnEntityPreDestroyed.Detach(this);
			m_pScene->OnEntityAttached.Detach(this);
			m_pScene->OnEntityDetached.Detach(this);
			m_pScene->OnEntityVisibilityChanged.Detach(this);
		}

		m_pScene = pScene;

		m_pScene->OnEntityCreated.Connect(this, &OutlinerPanel::OnEntityCreated);
		m_pScene->OnEntityPreDestroyed.Connect(this, &OutlinerPanel::OnEntityPreDestroyed);
		m_pScene->OnEntityAttached.Connect(this, &OutlinerPanel::OnEntityAttached);
		m_pScene->OnEntityDetached.Connect(this, &OutlinerPanel::OnEntityDetached);
		m_pScene->OnEntityVisibilityChanged.Connect(this, &OutlinerPanel::OnEntityVisibilityChanged);

		m_pOutliner->RemoveAll();

		m_pScene->GetEntityManager().Collect<RootComponent>().Do([this](entity e)
			{
				std::function<void(entity entityID, const std::shared_ptr<OutlinerEntityTreeItem>& pParentOutlinerEntityTreeItem)> RecursiveAddEntityTreeItem
					 = [&](entity entityID, const std::shared_ptr<OutlinerEntityTreeItem>& pParentOutlinerEntityTreeItem)
				{
					const std::shared_ptr<OutlinerEntityTreeItem> pNewEntityTreeItem = CreateEntityTreeItem(entityID);
					
					if (pParentOutlinerEntityTreeItem)
						pParentOutlinerEntityTreeItem->AddChild(pNewEntityTreeItem);
					else
						m_pOutliner->AddEntry(pNewEntityTreeItem);

					if (m_pScene->GetEntityManager().Has<ParentComponent>(entityID))
					{
						const std::vector<entity>& children = m_pScene->GetEntityManager().Get<ParentComponent>(entityID).Children;
						for (auto child : children)
							RecursiveAddEntityTreeItem(child, pNewEntityTreeItem);
					}
				};

				RecursiveAddEntityTreeItem(e, nullptr);
			});
	}

	void OutlinerPanel::OnEntityCreated(entity e) noexcept
	{
		const std::shared_ptr<OutlinerEntityTreeItem> pEntityTreeItem = CreateEntityTreeItem(e);
		m_pOutliner->AddEntry(pEntityTreeItem);
	}

	void OutlinerPanel::OnEntityPreDestroyed(entity e) noexcept
	{
		TreeItem* pParentTreeItem = m_EntityToTreeItemMap[e]->GetParent();
		const std::vector<std::shared_ptr<TreeItem>>& children = m_EntityToTreeItemMap[e]->GetChildren();
		
		if (pParentTreeItem)
		{
			pParentTreeItem->RemoveChild(m_EntityToTreeItemMap[e]);

			for (auto& child : children)
				pParentTreeItem->AddChild(child);
		}
		else
		{
			m_pOutliner->RemoveEntry(m_EntityToTreeItemMap[e]);

			for (auto& child : children)
			{
				child->RemoveParent();
				m_pOutliner->AddEntry(child);
			}
		}

		m_EntityToTreeItemMap.erase(e);
	}

	void OutlinerPanel::OnEntityAttached(entity child, entity parent) noexcept
	{
		std::shared_ptr<OutlinerTreeItem> pChildTreeItem = m_EntityToTreeItemMap[child];
		std::shared_ptr<OutlinerTreeItem> pParentTreeItem = m_EntityToTreeItemMap[parent];

		std::vector<std::shared_ptr<TreeItem>>& rootEntries = m_pOutliner->GetRootEntries();

		rootEntries.erase(
			std::remove_if(
				rootEntries.begin(),
				rootEntries.end(),
				[&pChildTreeItem](const std::shared_ptr<TreeItem>& item) {
					return item == pChildTreeItem;
				}),
			rootEntries.end());
		
		pParentTreeItem->AddChild(pChildTreeItem);
	}

	void OutlinerPanel::OnEntityDetached(entity child, entity parent) noexcept
	{

	}

	void OutlinerPanel::OnEntitySelectionChangedFromEditor(entity e, ESelectionState selectionState) noexcept
	{
		std::shared_ptr<OutlinerTreeItem> pEntityTreeItem = m_EntityToTreeItemMap[e];

		switch (selectionState)
		{
		case ESelectionState::Selected:
		{
			pEntityTreeItem->SetBackgroundColor(m_pOutliner->IsFocused() ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
			pEntityTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
			break;
		}
		case ESelectionState::Deselected:
		{
			pEntityTreeItem->ResetBackgroundColor();
			const ImVec4 iconTint = pEntityTreeItem->IsVisible() ? ImVec4(1.0f, 1.0f, 1.0f, 0.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
			pEntityTreeItem->SetIconTint(0u, iconTint);
			break;
		}
		default:
		{
			RLS_ASSERT(false, "Unreachable");
			break;
		}
		}
	}

	void OutlinerPanel::OnEntityVisibilityChanged(entity e, bool visibilityState) noexcept
	{
		const bool isSelected = m_pEditor->GetSelection().IsEntitySelected(e);

		m_EntityToTreeItemMap[e]->SetVisibility(visibilityState);
		m_EntityToTreeItemMap[e]->SetVisibilityIcon(visibilityState ? m_ShowEntityTextureIconHandle : m_HideEntityTextureIconHandle);
		m_EntityToTreeItemMap[e]->SetIconTint(0u, !visibilityState ? ImVec4(1.0f, 1.0f, 1.0f, 0.6f) : (visibilityState && isSelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
	}

	void OutlinerPanel::OnOutlinerTreeFocusChanged(bool isFocused) noexcept
	{
		const std::vector<entity>& selectedEntities = m_pEditor->GetSelection().GetSelectedEntities();
		for (entity selectedEntity : selectedEntities)
			m_EntityToTreeItemMap[selectedEntity]->SetBackgroundColor(isFocused ? TreeDefaultColors::RowSelectedFocusedColor : TreeDefaultColors::RowSelectedColor);
	}

	std::shared_ptr<OutlinerEntityTreeItem> OutlinerPanel::CreateEntityTreeItem(entity e) noexcept
	{
		std::shared_ptr<OutlinerEntityTreeItem> pEntityTreeItem = std::make_shared<OutlinerEntityTreeItem>(e);

		TreeItemData data;
		data.ColumnLabels.resize(3);

		data.ColumnLabels[0] = "";
		data.ColumnLabels[1] = m_pScene->GetEntityManager().Get<NameComponent>(e).Name;
		data.ColumnLabels[2] = "Entity";

		data.ColumnIcons.resize(3);
		data.ColumnIcons[0].IconTextureHandle = m_ShowEntityTextureIconHandle;
		data.ColumnIcons[0].Tint = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

		data.ColumnStyles.resize(3);
		data.ColumnStyles[0].Alignment = UI::Alignment::Center;

		data.UseDefaultItemColor = true;

		pEntityTreeItem->SetData(data);

		m_EntityToTreeItemMap[e] = pEntityTreeItem;

		return pEntityTreeItem;
	}

	void OutlinerPanel::OnTreeItemClicked(std::shared_ptr<TreeItem> pTreeItem, uint32_t column, bool doubleClicked) noexcept
	{
		OutlinerTreeItem* pItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);

		if (doubleClicked && pItem->GetType() == ETreeItemType::Filter)
			pItem->SetExpanded(!pItem->IsExpanded());
		else
		{
			const EColumnType columnType = static_cast<EColumnType>(column);

			if (columnType == EColumnType::Visibility)
			{
				std::vector<std::shared_ptr<TreeItem>> treeItemsToEditVisibilityFor = GetAllSelectedTreeItems();
				if (!IsTreeItemSelected(pItem))
					treeItemsToEditVisibilityFor.push_back(pTreeItem);

				const bool newVisibilityState = !pItem->IsVisible();

				for (auto& treeItem : treeItemsToEditVisibilityFor)
				{
					OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);
					switch (pOutlinerTreeItem->GetType())
					{
					case ETreeItemType::Entity:
					{
						const entity e = OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem)->GetEntityID();
						m_pScene->SetEntityVisibleInGame(e, newVisibilityState);
						break;
					}
					}

				}
			}
			else
				DetermineAndIssueSelection(pItem, column);
		}
	}

	void OutlinerPanel::OnTreeItemHovered(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept
	{
		const EColumnType columnType = static_cast<EColumnType>(column);

		switch (columnType)
		{
		case EColumnType::Visibility: UI::Utility::DrawTooltip("Toggles the visibility of this entity in the scene editor."); break;
		case EColumnType::Label: UI::Utility::DrawTooltip(pTreeItem->GetColumnLabel(column).c_str()); break;
		default: break;
		}

		if (!IsTreeItemSelected(pTreeItem.get()))
		{
			const ImVec4 visibilityIconTint = column == 0u ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
			pTreeItem->SetIconTint(0u, visibilityIconTint);
		}
	}

	void OutlinerPanel::OnMouseEnterTreeItemRow(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept
	{
		if (IsTreeItemSelected(pTreeItem.get()))
			return;
			
		pTreeItem->SetBackgroundColor(TreeDefaultColors::RowHoverColor);
	}

	void OutlinerPanel::OnMouseExitTreeItemRow(std::shared_ptr<TreeItem> pTreeItem) noexcept
	{
		const bool isVisible = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem)->IsVisible();
		if (!isVisible && !IsTreeItemSelected(pTreeItem.get()))
			pTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
		
		if (!IsTreeItemSelected(pTreeItem.get()))
		{
			if (isVisible)
				pTreeItem->SetIconTint(0u, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));

			pTreeItem->ResetBackgroundColor();
			std::cout << " - CALLED RESET FROM OnMouseExitTreeItemRow";
		}
	}

	void OutlinerPanel::OnMouseReleasedOnTreeItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept
	{
		if (column == 0u)
			return;

		if (!IsTreeItemSelected(pTreeItem.get()))
			return;
		
		if (GetNumSelected() <= 1)
			return;
		
		OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);

		switch (OutlinerPanel_private::GetSelectionMode())
		{
		case OutlinerPanel_private::SelectionMode::Single:
		{
			DeselectAllTreeItems();
			SelectTreeItem(pOutlinerTreeItem);
			SetReferenceSelection(pOutlinerTreeItem);
			break;
		}
		}
	}

	bool OutlinerPanel::OnDragOver(const Any& payload, const Any& target, std::string_view dragContext) noexcept
	{
		if (dragContext != "TreeItem")
		{
			RLS_CORE_INFO("IS FALSE");
			return false;
		}

		const std::shared_ptr<TreeItem> pPayload = *payload.Get<std::shared_ptr<TreeItem>>();
		const std::shared_ptr<TreeItem> pTarget = *target.Get<std::shared_ptr<TreeItem>>();

		const std::vector<std::shared_ptr<TreeItem>> selectedTreeItems = GetAllSelectedTreeItems();

		if (std::any_of(selectedTreeItems.begin(), selectedTreeItems.end(), [&](const std::shared_ptr<TreeItem>& pTreeItem)
			{
				return pTreeItem == pTarget;
			}))
		{
			m_DragDropTooltip = "Cannot attach entity to self";
			RLS_CORE_INFO("IS FALSE");

			return false;
		}

		OutlinerTreeItem* pTargetOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTarget);

		if (std::any_of(selectedTreeItems.begin(), selectedTreeItems.end(), [&](const std::shared_ptr<TreeItem>& pTreeItem)
			{
				OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(pTreeItem);
				return pOutlinerTreeItem->GetType() == ETreeItemType::Filter && pTargetOutlinerTreeItem->GetType() == ETreeItemType::Entity;
			}))
		{
			m_DragDropTooltip = "Cannot attach filters to entities.";
			RLS_CORE_INFO("IS FALSE");

			return false;
		}

		//Is pTarget the descendant of any of the selected? If so, return false
		//We ensure uniqueness with the unordered_set!

		std::unordered_set<std::shared_ptr<TreeItem>> allDescendants;
		for (auto& selected : selectedTreeItems)
		{
			const std::vector<std::shared_ptr<TreeItem>> descendants = m_pOutliner->GetDescendants(selected);
			allDescendants.insert(descendants.begin(), descendants.end());
		}

		if (std::any_of(allDescendants.begin(), allDescendants.end(), [&](const std::shared_ptr<TreeItem>& pDescendant)
			{
				return pDescendant == pTarget;
			}))
		{
			m_DragDropTooltip = "Parent cannot become the child of their descendant.";
			RLS_CORE_INFO("IS FALSE");

			return false;
		}

		m_DragDropTooltip = pTarget->GetColumnLabel(1);

		m_DraggedOnValidTargetThisFrame = true;
		RLS_CORE_INFO("IS TRUE");
		return true;
	}

	void OutlinerPanel::OnDrop(const Any& payload, const Any& target, std::string_view dropContext) noexcept
	{
		if (dropContext != "TreeItem")
			return;

		OutlinerTreeItem* pTargetTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(*target.Get<std::shared_ptr<TreeItem>>());
		if (pTargetTreeItem->GetType() != ETreeItemType::Entity)
			return;

		OutlinerEntityTreeItem* pTargetEntityTreeItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pTargetTreeItem);

		std::vector<OutlinerEntityTreeItem*> entityTreeItems;

		const std::vector<std::shared_ptr<TreeItem>> allSelectedTreeItems = GetAllSelectedTreeItems();
		for (auto& selectedTreeItem : allSelectedTreeItems)
		{
			OutlinerTreeItem* pOutlinerTreeItem = OutlinerPanel_private::AsOutlinerTreeItem(selectedTreeItem);
			switch (pOutlinerTreeItem->GetType())
			{
			case ETreeItemType::Entity:
			{
				entityTreeItems.push_back(OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem));
				break;
			}
			default:
				break;
			}
		}

		for (OutlinerEntityTreeItem* pEntityTreeItem : entityTreeItems)
		{
			m_pScene->AttachEntity(pEntityTreeItem->GetEntityID(), pTargetEntityTreeItem->GetEntityID());
		}

	}

	void OutlinerPanel::SetAndPropagateTreeItemVisibility(OutlinerTreeItem* pOutlinerTreeItem, bool visibilityState) noexcept
	{
		const bool isSelected = IsTreeItemSelected(pOutlinerTreeItem);

		pOutlinerTreeItem->SetVisibility(visibilityState);
		pOutlinerTreeItem->SetVisibilityIcon(visibilityState ? m_ShowEntityTextureIconHandle : m_HideEntityTextureIconHandle);
		
		if (pOutlinerTreeItem->GetType() == ETreeItemType::Entity)
		{
			const entity e = OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem)->GetEntityID();
			EntityManager& entityManager = m_pScene->GetEntityManager();
			
			if (visibilityState == true)
			{
				if (entityManager.Has<HiddenInGameComponent>(e))
					entityManager.Remove<HiddenInGameComponent>(e);
			}
			else
				entityManager.AddOrReplace<HiddenInGameComponent>(e);
		}

		const std::vector<std::shared_ptr<TreeItem>>& children = pOutlinerTreeItem->GetChildren();
		
		if (isSelected)
			pOutlinerTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
		else
		{
			if (visibilityState == false)
				pOutlinerTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 0.6f));
			else
				pOutlinerTreeItem->SetIconTint(0u, ImVec4(1.0f, 1.0f, 1.0f, 0.0f));
		}

		for (auto& child : children)
			SetAndPropagateTreeItemVisibility(static_cast<OutlinerTreeItem*>(child.get()), visibilityState);
	}

	void OutlinerPanel::SetReferenceSelection(OutlinerTreeItem* pReferenceSelection)
	{
		m_pReferenceSelection = pReferenceSelection;
	}

	void OutlinerPanel::DetermineAndIssueSelection(OutlinerTreeItem* pTreeItem, uint32_t clickedColumn) noexcept
	{
		using namespace OutlinerPanel_private;

		const bool isSelected = IsTreeItemSelected(pTreeItem);

		switch (GetSelectionMode())
		{
		case SelectionMode::Single:
		{
			if (GetNumSelected() > 1 && isSelected)
				return;

			if (IsTreeItemSelectableAtClickedColumn(pTreeItem, clickedColumn))
			{
				DeselectAllTreeItems();
				SelectTreeItem(pTreeItem);
				SetReferenceSelection(pTreeItem);
			}

			break;
		}
		case SelectionMode::Toggle:
		{
			if (IsTreeItemSelected(pTreeItem))
			{
				DeselectTreeItem(pTreeItem);
				if (GetNumSelected() > 0)
				{
					switch (m_LastSelectedType)
					{
					case ETreeItemType::Entity:
					{
						const entity firstSelected = m_pEditor->GetSelection().GetFirstSelected();
						RLS_ASSERT(firstSelected != NULL_ENTITY, "[OutlinerPanel]: Invalid enitity ID!");
						SetReferenceSelection(m_EntityToTreeItemMap[firstSelected].get());
						break;
					}
					default:
						break;
					}
				}
			}
			else
			{
				if (IsTreeItemSelectableAtClickedColumn(pTreeItem, clickedColumn))
				{
					SelectTreeItem(pTreeItem);
					SetReferenceSelection(pTreeItem);
				}
			}
			break;
		}
		case SelectionMode::Range:
		{
			const std::vector<Tree::TreeDataRow> treeDatas = m_pOutliner->FlattenTree();
			if (treeDatas.empty())
				break;

			size_t startIndex = 0u;
			if (m_pReferenceSelection)
			{
				auto it = std::find_if(treeDatas.begin(), treeDatas.end(), [&](const Tree::TreeDataRow& row)
					{
						return row.Entry.get() == m_pReferenceSelection;
					});
				startIndex = std::distance(treeDatas.begin(), it);
			}
			else
				m_pReferenceSelection = treeDatas[0].Entry.get();

			auto it = std::find_if(treeDatas.begin(), treeDatas.end(), [&](const Tree::TreeDataRow& row)
				{
					return row.Entry.get() == pTreeItem;
				});

			size_t endIndex = std::distance(treeDatas.begin(), it);

			if (startIndex > endIndex)
				std::swap(startIndex, endIndex);

			for (size_t i = startIndex; i <= endIndex; ++i)
			{
				if (IsTreeItemSelectableAtClickedColumn(treeDatas[i].Entry.get(), clickedColumn) && !IsTreeItemSelected(treeDatas[i].Entry.get()))
					SelectTreeItem(static_cast<OutlinerTreeItem*>(treeDatas[i].Entry.get()));
			}

			break;
		}
		}
	}

	void OutlinerPanel::SelectTreeItem(OutlinerTreeItem* pTreeItem) noexcept
	{
		switch (pTreeItem->GetType())
		{
		case ETreeItemType::Entity:
		{
			m_pEditor->GetSelection().SelectEntity(OutlinerPanel_private::AsOutlinerEntityTreeItem(pTreeItem)->GetEntityID());
			break;
		}
		}
	}

	void OutlinerPanel::DeselectTreeItem(OutlinerTreeItem* pTreeItem) noexcept
	{
		switch (pTreeItem->GetType())
		{
		case ETreeItemType::Entity:
		{
			OutlinerEntityTreeItem* pEntityTreeItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pTreeItem);
			m_pEditor->GetSelection().DeselectEntity(pEntityTreeItem->GetEntityID());
			break;
		}
		}
	}

	void OutlinerPanel::DeselectAllTreeItems() noexcept
	{
		m_pEditor->GetSelection().DeselectAllEntities();
	}

	bool OutlinerPanel::IsTreeItemSelected(TreeItem* pTreeItem) const noexcept
	{
		OutlinerTreeItem* pOutlinerTreeItem = static_cast<OutlinerTreeItem*>(pTreeItem);

		switch (pOutlinerTreeItem->GetType())
		{
		case ETreeItemType::Entity: 
		{
			OutlinerEntityTreeItem* pEntityItem = OutlinerPanel_private::AsOutlinerEntityTreeItem(pOutlinerTreeItem);
			return m_pEditor->GetSelection().IsEntitySelected(pEntityItem->GetEntityID());
		}
		default:
			return false;
		}
	}

	bool OutlinerPanel::IsTreeItemSelectableAtClickedColumn(TreeItem* pTreeItem, uint32_t column) const noexcept
	{
		if (column == 0)
			return false;

		return true;
	}

	std::vector<std::shared_ptr<TreeItem>> OutlinerPanel::GetAllSelectedTreeItems() const noexcept
	{
		std::vector<std::shared_ptr<TreeItem>> selectedTreeItems;
		selectedTreeItems.reserve(GetNumSelected());

		const std::vector<entity>& selectedEntities = m_pEditor->GetSelection().GetSelectedEntities();

		for (const entity selectedEntity : selectedEntities)
			selectedTreeItems.push_back(m_EntityToTreeItemMap.at(selectedEntity));

		return selectedTreeItems;
	}

	uint32_t OutlinerPanel::GetNumSelected() const noexcept
	{
		return m_pEditor->GetSelection().GetSelectedEntityCount();
	}

	//bool OutlinerPanel::DrawTableEntry(entity currentEntity) noexcept
	//{
	//	ImGui::TableNextRow();
	//	
	//	bool isHoveringRow = false;
	//	ImRect cellRects[3];
	//	
	//	for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
	//	{
	//		ImGui::TableSetColumnIndex(column);
	//	
	//		cellRects[column] = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
	//		cellRects[column].Max.y += ImGui::TableGetHeaderRowHeight() - 6.0f;
	//	
	//		isHoveringRow |= ImGui::IsMouseHoveringRect(cellRects[column].Min, cellRects[column].Max);
	//	}
	//	
	//	const bool entityIsSelected = IsEntitySelected(currentEntity);
	//	if (entityIsSelected)
	//	{
	//		for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
	//		{
	//			const ImU32 bgColor = m_IsTableFocused ? IM_COL32(30, 120, 255, 200) : IM_COL32(64.0f, 87.0f, 111.0f, 255.0f);
	//	
	//			ImGui::TableSetColumnIndex(column);
	//			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(bgColor));
	//		}
	//	}
	//	else if (isHoveringRow)
	//	{
	//		for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
	//		{
	//			ImGui::TableSetColumnIndex(column);
	//	
	//			constexpr ImVec4 hoverColor = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
	//			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(hoverColor));
	//		}
	//	}
	//	else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity entityToTestForDescendancy)
	//		{
	//			return m_pScene->EntityIsDescendant(currentEntity, entityToTestForDescendancy);
	//		}))
	//	{
	//		constexpr ImU32 bgColor = IM_COL32(44.0f, 50.0f, 58.0f, 255.0f);
	//		for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
	//		{
	//			ImGui::TableSetColumnIndex(column);
	//			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(bgColor));
	//		}
	//	}
	//	
	//	if (isHoveringRow)
	//		m_HoveredEntity = currentEntity;
	//	
	//	EntityManager& entityManager = m_pScene->GetEntityManager();
	//	
	//	const bool isHiddenInGame = entityManager.Has<HiddenInGameComponent>(currentEntity);
	//	
	//	ImGui::TableSetColumnIndex(0);
	//	
	//	const bool hoversColumn0 = ImGui::IsMouseHoveringRect(cellRects[0].Min, cellRects[0].Max);
	//	
	//	const uint32_t nrOfAncestors = m_pScene->GetAllEntityAncestors(currentEntity).size();
	//	
	//	const float indentation = ImGui::GetTreeNodeToLabelSpacing() * (nrOfAncestors + 1);
	//	ImGui::Unindent(indentation);
	//	if (isHoveringRow || isHiddenInGame || entityIsSelected)
	//	{
	//		constexpr ImVec2 imageSize = ImVec2(22, 12);
	//		constexpr float imagePositionPadding = 3.0f;
	//		const ImVec4 imageTint = (hoversColumn0 || entityIsSelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	//	
	//		const float availableWidth = ImGui::GetContentRegionAvail().x;
	//		const float availableHeight = ImGui::GetTextLineHeightWithSpacing();
	//		const float offsetX = (availableWidth - imageSize.x) * 0.5f;
	//		const float offsetY = (availableHeight - imageSize.y) * 0.5f;
	//	
	//		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX + imagePositionPadding);
	//		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY - imagePositionPadding);
	//	
	//		const std::shared_ptr<Texture2D> visibilityIconTexture = isHiddenInGame ? AssetManager::Get<Texture2D>(m_HideEntityTextureIconHandle) : AssetManager::Get<Texture2D>(m_ShowEntityTextureIconHandle);
	//		ImGui::Image((ImTextureID)visibilityIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, imageSize, ImVec2(0, 0), ImVec2(1, 1), imageTint);
	//	
	//		if (hoversColumn0)
	//			UI::Utility::DrawTooltip("Toggles the visibility of this entity in the level editor.");
	//	
	//		if (hoversColumn0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	//		{
	//			if (isHiddenInGame)
	//				entityManager.Remove<HiddenInGameComponent>(currentEntity);
	//			else
	//				entityManager.Add<HiddenInGameComponent>(currentEntity);
	//		}
	//	}
	//	ImGui::Indent(indentation);
	//	
	//	ImGui::TableSetColumnIndex(1);
	//	
	//	auto& nameComponent = entityManager.Get<NameComponent>(currentEntity);
	//	
	//	ImGuiStyle& style = ImGui::GetStyle();
	//	const ImVec4 originalHoverColor = style.Colors[ImGuiCol_HeaderHovered];
	//	const ImVec4 originalActiveColor = style.Colors[ImGuiCol_HeaderActive];
	//	
	//	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	//	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	//	
	//	const bool isParent = entityManager.Has<ParentComponent>(currentEntity);
	//	ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;
	//	if (isParent)
	//		nodeFlags |= (ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
	//	else
	//		nodeFlags |= ImGuiTreeNodeFlags_Leaf;
	//	
	//	ImGui::Indent(indentation);
	//	const bool isOpen = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)currentEntity, nodeFlags, nameComponent.Name.c_str());
	//	bool hoversName = ImGui::IsItemHovered();
	//	
	//	ImGui::Unindent(indentation);
	//	
	//	style.Colors[ImGuiCol_HeaderHovered] = originalHoverColor;
	//	style.Colors[ImGuiCol_HeaderActive] = originalActiveColor;
	//	
	//	if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
	//	{
	//		m_DraggedEntity = currentEntity;
	//	
	//		ImGui::SetDragDropPayload("OUTLINER_TABLE_PAYLOAD", &currentEntity, sizeof(entity), ImGuiCond_::ImGuiCond_Once);
	//		ImGui::EndDragDropSource();
	//	}
	//	if (ImGui::BeginDragDropTarget())
	//	{
	//		if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("OUTLINER_TABLE_PAYLOAD"))
	//		{
	//			const entity toBecomeChild = *(entity*)payLoad->Data;
	//	
	//			if (std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
	//				{
	//					return e != currentEntity && !m_pScene->EntityIsDescendant(e, currentEntity);
	//				}))
	//			{
	//				//We know ALL entities are not the one being dragged upon and we know the target for drag drop is not child of any dragged entity.
	//				//Now we either attach or detach.
	//
	//				//Rule 1: if ALL selected entities are direct children of targetted entity, we should detach all of them outright:
	//				if (std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
	//					{
	//						return m_pScene->EntityIsChild(e, currentEntity);
	//					}))
	//				{
	//					std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e)
	//						{
	//							m_pScene->DetachEntity(e);
	//						});
	//				} //Rule 2: If any selected entity is direct child of target entity, ALL entities remain as or become direct children of targetted entity.
	//				else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
	//					{
	//						return m_pScene->EntityIsChild(e, currentEntity);
	//					}))
	//				{
	//					std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
	//						{
	//							if (m_pScene->EntityIsChild(e, currentEntity))
	//							return;
	//
	//					m_pScene->AttachEntity(e, currentEntity);
	//						});
	//				}
	//				else //Just regular attach all:
	//				{
	//					std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
	//						{
	//							m_pScene->AttachEntity(e, currentEntity);
	//						});
	//				}
	//			}
	//		}
	//		ImGui::EndDragDropTarget();
	//	}
	//	
	//	const bool hoversColumn1 = ImGui::IsMouseHoveringRect(cellRects[1].Min, cellRects[1].Max);
	//	
	//	ImRect collapseArrowRect;
	//	collapseArrowRect.Min = cellRects[1].Min;
	//	collapseArrowRect.Min.x += indentation;
	//	collapseArrowRect.Min.x += ImGui::GetStyle().FramePadding.x;
	//	collapseArrowRect.Min.x += 6.0f;
	//	collapseArrowRect.Max = ImVec2(collapseArrowRect.Min.x, cellRects[1].Max.y);
	//	collapseArrowRect.Max.x += ImGui::GetTreeNodeToLabelSpacing();
	//	collapseArrowRect.Max.x -= 9.0f;
	//	const bool hoversCollapseArrowRect = ImGui::IsMouseHoveringRect(collapseArrowRect.Min, collapseArrowRect.Max);
	//	ImGui::GetWindowDrawList()->AddRect(collapseArrowRect.Min, collapseArrowRect.Max, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
	//	
	//	ImGui::TableSetColumnIndex(2);
	//	
	//	ImGui::Text("Entity");
	//	const bool hoversColumn2 = ImGui::IsMouseHoveringRect(cellRects[2].Min, cellRects[2].Max);
	//	
	//	const bool isHoveringSelectableRowSpace = isParent ? (hoversColumn1 && !hoversCollapseArrowRect) : hoversColumn1;
	//	
	//	if ((isHoveringSelectableRowSpace || hoversColumn2) && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
	//		OnMouseReleasedOverTableEntry();
	//	else if ((isHoveringSelectableRowSpace || hoversColumn2) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
	//		OnTableEntryClicked();
	//	
	//	if (isOpen && entityManager.Has<ParentComponent>(currentEntity))
	//	{
	//		auto& pc = entityManager.Get<ParentComponent>(currentEntity);
	//		for (entity child : pc.Children)
	//			isHoveringRow |= DrawTableEntry(child);
	//	}
	//	
	//	if (isOpen)
	//		ImGui::TreePop();
	//
	//	return isHoveringRow;
	//}

	//void OutlinerPanel::DrawDraggingTooltip() noexcept
	//{
	//	if (!ImGui::IsDragDropActive())
	//		return;
	//	
	//	if (!m_pScene)
	//		return;
	//	
	//	const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_TABLE_PAYLOAD", ImGuiDragDropFlags_AcceptPeekOnly);
	//	if (!payload)
	//		return;
	//	
	//	const entity toBecomeChild = *(entity*)payload->Data;
	//	EntityManager& entityManager = m_pScene->GetEntityManager();
	//	
	//	if (m_HoveredEntity != NULL_ENTITY)
	//	{
	//		auto& nameComponent = entityManager.Get<NameComponent>(m_HoveredEntity);
	//	
	//		if (std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager, this](entity e)
	//			{
	//				return entityManager.Has<IsChildComponent>(e) && (entityManager.Get<IsChildComponent>(e).Parent == m_HoveredEntity); //Entity is parent?
	//			}))
	//		{
	//			ImGui::SetTooltip("Detach from %s.", nameComponent.Name.c_str());
	//		}
	//		else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e)
	//			{
	//				return e == m_HoveredEntity;
	//			}))
	//		{
	//			ImGui::SetTooltip("Cannot attach entity to self.", nameComponent.Name.c_str());
	//		}
	//		else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e)
	//			{
	//				return m_pScene->EntityIsDescendant(e, m_HoveredEntity);
	//			}))
	//		{
	//			ImGui::SetTooltip("Parent entity cannot become the child of their descendant.");
	//		}
	//		else
	//			ImGui::SetTooltip("Attach to %s.", nameComponent.Name.c_str());
	//	}
	//	else
	//	{
	//		if (m_SceneTableEntryIsHovered || m_TableEmptySpaceHovered)
	//		{
	//			if (auto it = std::find_if(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager](entity e)
	//				{
	//					return entityManager.Has<RootComponent>(e);
	//				}); it != m_SelectedEntities.end())
	//			{
	//				ImGui::SetTooltip("%s is already attached to root.", entityManager.Get<NameComponent>(*it).Name.c_str());
	//			}
	//			else
	//				ImGui::SetTooltip("Move to root.");
	//		}
	//		else
	//		{
	//			std::string tooltip = entityManager.Get<NameComponent>(m_DraggedEntity).Name;
	//			if (m_SelectedEntities.size() > 1)
	//				tooltip += std::format(" and {} other{}.", m_SelectedEntities.size() - 1, (m_SelectedEntities.size() - 1) > 1 ? "s" : "");
	//	
	//			ImGui::SetTooltip(tooltip.c_str());
	//		}
	//	}
	//}	

	bool OutlinerPanel::IsAncestorToAnySelected(const std::shared_ptr<TreeItem>& treeItem, const std::vector<std::shared_ptr<TreeItem>>& selected) const noexcept
	{
		std::vector<std::shared_ptr<TreeItem>> decendants;
	
		std::function<void(const std::shared_ptr<TreeItem>&)> GetChildren;
	
		GetChildren = [&](const std::shared_ptr<TreeItem>& currentTreeItem)
		{
			const std::vector<std::shared_ptr<TreeItem>> children = currentTreeItem->GetChildren();
			for (auto& child : children)
			{
				decendants.push_back(child);
				GetChildren(child);
			}
		};
	
		GetChildren(treeItem);
	
		return std::any_of(selected.begin(), selected.end(), [&](const std::shared_ptr<TreeItem>& pCurrentTreeItem) -> bool
			{
				return std::any_of(decendants.begin(), decendants.end(), [&pCurrentTreeItem](const std::shared_ptr<TreeItem>& pCurrentData)
					{
						return pCurrentTreeItem == pCurrentData;
					});
			});
	}
}