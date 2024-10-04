#include "SceneHierarchyPanel.h"
#include "../Core/EditorLayer.h"
namespace Relentless
{
	namespace SceneHierarchyPanel_private
	{
		constexpr int TABLE_COLUMN_COUNT = 3;
	}

	SceneHierarchyPanel::SceneHierarchyPanel() noexcept
		: m_pScene{ nullptr },
		  m_SelectedEntity{ NULL_ENTITY }
	{
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\showicon.rasset", m_ShowEntityTextureIconHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\hideicon.rasset", m_HideEntityTextureIconHandle), "Core engine icon missing.");
	}

	void SceneHierarchyPanel::OnEvent(IEvent& event)
	{
		switch (event.GetEventType())
		{
		case EventType::KeyPressedEvent:
		{
			const RLS_KEY key = EVENT(KeyPressedEvent).key;
			switch (key)
			{
			case RLS_KEY::A:
			{
				if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
				{
					m_Table.SelectAllExpandedEntityRows();
					event.StopPropagation();
				}
				break;
			}
			}
		}
		}
	}

	void SceneHierarchyPanel::OnImGuiRender(const bool show) noexcept
	{
		if (!show)
			return;
		
		PROFILE_FUNC;
		
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Outliner");
		ImGui::PopStyleVar();
		
		constexpr float bannerHeight = 40.0f;

		{
			ImGui::BeginChild("Outliner SearchBar Child", ImVec2(0.0f, 50.0f));
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
			m_ContentFilter = UI::SearchBar("OutlinerSearchBar", "Search...", true);
			ImGui::EndChild();
		}

		{
			ImGui::BeginChild("Outliner Table Child", ImVec2(0, ImGui::GetContentRegionAvail().y - bannerHeight));
			m_Table.Draw();

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !m_Table.IsHovered())
				m_Table.ClearAllSelections();

			ImGui::EndChild();
		}

		{
			ImGui::BeginChild("Outliner Banner Child");
			
			const ImVec2 bannerStartPos = ImGui::GetCursorScreenPos();
			const ImVec2 availableSpace = ImGui::GetContentRegionAvail();
			const ImVec2 bannerEndPos = ImVec2(bannerStartPos.x + availableSpace.x, bannerStartPos.y + availableSpace.y);

			ImDrawList* pDrawList = ImGui::GetWindowDrawList();
			pDrawList->AddRectFilled(bannerStartPos, bannerEndPos, ImGui::ColorConvertFloat4ToU32(ImVec4(0.2f, 0.2f, 0.2f, 1.0f)));

			const uint32_t numEntities = m_Table.GetNrOfEntityEntries();
			const uint32_t selectedEntities = m_Table.GetNrOfSelectedEntities();

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
		return;









		m_HoveredEntity = NULL_ENTITY;

		EntityManager& entityManager = m_pScene->GetEntityManager();

		for (uint32_t i = 0u; i < m_SelectedEntities.size(); ++i)
		{
			if (!entityManager.Exists(m_SelectedEntities[i]))
				m_SelectedEntities.erase(m_SelectedEntities.begin() + i);
		}

		m_SceneIsHiddenInGame = entityManager.GetEntityCountForPool<HiddenInGameComponent>() == entityManager.GetEntityAliveCount();

		
		constexpr ImVec4 evenTableRowBgColor = ImVec4(21.0f / 255.0f, 21.0f / 255.0f, 21.0f / 255.0f, 255.0f / 255.0f);
		constexpr ImVec4 oddTableRowBgColor = ImVec4(26.0f / 255.0f, 26.0f / 255.0f, 26.0f / 255.0f, 255.0f / 255.0f);
		
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBg, evenTableRowBgColor);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBgAlt, oddTableRowBgColor);
		
		constexpr ImGuiTableFlags flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
		
		bool hoversAnyRow = false;
		
		if (ImGui::BeginTable("OutlinerTable", SceneHierarchyPanel_private::TABLE_COLUMN_COUNT, flags))
		{
			m_IsTableFocused = ImGui::IsWindowFocused();
		
			ImGui::TableSetupScrollFreeze(0, 1);
		
			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 30.0f);
			ImGui::TableSetupColumn("Item Label", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();
		
			for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
			{
				ImGui::TableSetColumnIndex(column);
				
				const ImRect headerRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
				if (ImGui::IsMouseHoveringRect(headerRect.Min, headerRect.Max))
				{
					const char* tooltip = (column == 0) ? "Visibility" : (column == 1) ? "Item Label" : "Displays the name of each entity's type";
					UI::Utility::DrawTooltip(tooltip);
				}
		
				if (column == 0)
				{
					constexpr ImVec2 imageSize = ImVec2(22, 12);
					constexpr float imagePositionPadding = 3.0f;
					constexpr ImVec4 imageTint = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
		
					const float availableWidth = ImGui::GetContentRegionAvail().x;
					const float availableHeight = ImGui::GetTextLineHeightWithSpacing();
					const float offsetX = (availableWidth - imageSize.x) * 0.5f;
					const float offsetY = (availableHeight - imageSize.y) * 0.5f;
		
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX + imagePositionPadding);
					ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY - imagePositionPadding);
					
					const std::shared_ptr<Texture2D> pShowIconTexture = AssetManager::Get<Texture2D>(m_ShowEntityTextureIconHandle);
					ImGui::Image((ImTextureID)pShowIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, imageSize, ImVec2(0,0), ImVec2(1,1), imageTint);
				}
				else
					ImGui::TableHeader(ImGui::TableGetColumnName(column));
			}
		
			bool isOpen = false;
			hoversAnyRow |= DrawTableRootEntry(isOpen);
		
			if (isOpen)
			{
				m_pScene->GetEntityManager().Collect<NameComponent>().Do([&](entity currentEntity, const NameComponent& nameComponent)
					{
						if (!m_pScene->GetEntityManager().Has<RootComponent>(currentEntity))
							return;
		
						const bool IsUsingFilter = !m_ContentFilter.empty();
						if (IsUsingFilter)
						{
							std::vector<entity> entititesToCheck = m_pScene->GetAllEntityDescendants(currentEntity);
							entititesToCheck.push_back(currentEntity);
		
							EntityManager& entityManager = m_pScene->GetEntityManager();
							std::string contentFilterToLower = m_ContentFilter;
							std::transform(contentFilterToLower.begin(), contentFilterToLower.end(), contentFilterToLower.begin(), ::tolower);
		
							if (std::all_of(entititesToCheck.begin(), entititesToCheck.end(), [this, &entityManager, &contentFilterToLower](entity entityToCheck)
								{
									auto& nc = entityManager.Get<NameComponent>(entityToCheck);
									std::string entryStemToLower = nc.Name;
									std::transform(entryStemToLower.begin(), entryStemToLower.end(), entryStemToLower.begin(), ::tolower);
		
									return entryStemToLower.find(contentFilterToLower) == std::string::npos;
								}))
							{
								return;
							}
						}
		
						hoversAnyRow |= DrawTableEntry(currentEntity);
					});
				ImGui::TreePop();
			}
			ImGui::EndTable();
		}
		
		if (!hoversAnyRow)
			m_HoveredEntity = NULL_ENTITY;
		
		DrawDraggingTooltip();
		
		constexpr float padding = 7.0f;
		const float currentYPosition = ImGui::GetCursorPosY();
		const ImVec2 windowSize = ImGui::GetWindowSize();
		const float remainingWindowHeight = windowSize.y - currentYPosition;
		
		ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX(), currentYPosition - padding));
		
		if (remainingWindowHeight > 0.0f)
		{
			if (ImGui::InvisibleButton("OUTLINER_EMPTY_DROP_AREA", ImVec2(ImGui::GetContentRegionAvail().x, remainingWindowHeight)))
			{
				m_SelectedEntities.clear();
				m_SceneTableEntrySelected = false;
			}
		
			m_TableEmptySpaceHovered = ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem);
		
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("OUTLINER_TABLE_PAYLOAD"))
				{
					const entity entityToDetach = *(entity*)payLoad->Data;
			
					EntityManager& entityManager = m_pScene->GetEntityManager();
			
					if (!std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager](entity e)
						{
							return entityManager.Has<RootComponent>(e);
						}))
					{
						std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e1)
							{
								if (!std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, e1](entity e2)
									{
										return m_pScene->EntityIsParent(e1, e2);
									}))
								{
									m_pScene->DetachEntity(e1);
								}
							});
					}
					ImGui::EndDragDropTarget();
				}
			}
		}
		
		ImGui::PopStyleColor(2);
		ImGui::End();

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

	void SceneHierarchyPanel::SetActiveScene(Scene* const pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is nullptr.");
		m_pScene = pScene;
		m_SelectedEntity = NULL_ENTITY;

		m_Table.OnSceneChanged(pScene);
	}

	void SceneHierarchyPanel::SetSelectedEntity(const entity entityID) noexcept
	{
		m_SelectedEntity = entityID;
	}

	void SceneHierarchyPanel::SetOnEntityDestroyFunction(std::function<void(entity id)> callBackFunction) noexcept
	{
		m_OnEntityDestroyedCallBack = callBackFunction;
	}

	void SceneHierarchyPanel::SetOnEntityCreatedFunction(std::function<void(entity id)> callBackFunction) noexcept
	{
		m_OnEntityCreatedCallBack = callBackFunction;
	}

	void SceneHierarchyPanel::SetOnEntitySelectedFunction(std::function<void(entity id)> callBackFunction) noexcept
	{
		m_OnEntitySelectedCallBack = callBackFunction;
	}

	bool SceneHierarchyPanel::IsEntitySelected(entity entity) const noexcept
	{
		return std::find(m_SelectedEntities.begin(), m_SelectedEntities.end(), entity) != m_SelectedEntities.end();
	}

	void SceneHierarchyPanel::DeselectEntity(entity entity) noexcept
	{
		RLS_ASSERT(IsEntitySelected(entity), "[SceneHierarchyPanel]: Entity to deselect is not currently selected.");

		auto newEnd = std::remove(m_SelectedEntities.begin(), m_SelectedEntities.end(), entity);
		m_SelectedEntities.erase(newEnd, m_SelectedEntities.end());
	}

	void SceneHierarchyPanel::SelectEntity(entity entity) noexcept
	{
		RLS_ASSERT(!IsEntitySelected(entity), "[SceneHierarchyPanel]: Entity to select is already selected.");
		m_SelectedEntities.push_back(entity);
	}

	void SceneHierarchyPanel::OnTableEntryClicked() noexcept
	{
		const bool lCtrlPressed = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
		const bool lShiftPressed = Keyboard::IsKeyPressed(RLS_KEY::LShift);

		if (IsEntitySelected(m_HoveredEntity))
		{
			if (lCtrlPressed)
				DeselectEntity(m_HoveredEntity);
			else
			{
				if (m_SelectedEntities.size() <= 1) //If > 1 a select is done through mouse release instead.
				{
					m_SelectedEntities.clear();
					m_SceneTableEntrySelected = false;
					SelectEntity(m_HoveredEntity);
				}
			}
		}
		else
		{
			if (!lCtrlPressed)
			{
				m_SelectedEntities.clear();
				m_SceneTableEntrySelected = false;
			}
			
			SelectEntity(m_HoveredEntity);
		}
	}

	void SceneHierarchyPanel::OnMouseReleasedOverTableEntry() noexcept
	{
		if (m_SelectedEntities.size() <= 1)
			return;

		if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
			return;
		
		if (IsEntitySelected(m_HoveredEntity))
		{
			m_SelectedEntities.clear();
			m_SceneTableEntrySelected = false;
			SelectEntity(m_HoveredEntity);
		}
	}

	void SceneHierarchyPanel::OnSelectAll() noexcept
	{
		if (!m_pScene)
			return;

		m_SelectedEntities.clear();
		m_pScene->GetEntityManager().Collect<NameComponent>().Do([this](entity currentEntity)
			{
				m_SelectedEntities.push_back(currentEntity);
			});

		m_SceneTableEntrySelected = true;
	}

	bool SceneHierarchyPanel::DrawTableEntry(entity currentEntity) noexcept
	{
		ImGui::TableNextRow();
		
		bool isHoveringRow = false;
		ImRect cellRects[3];
		
		for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
		{
			ImGui::TableSetColumnIndex(column);
		
			cellRects[column] = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
			cellRects[column].Max.y += ImGui::TableGetHeaderRowHeight() - 6.0f;
		
			isHoveringRow |= ImGui::IsMouseHoveringRect(cellRects[column].Min, cellRects[column].Max);
		}
		
		const bool entityIsSelected = IsEntitySelected(currentEntity);
		if (entityIsSelected)
		{
			for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
			{
				const ImU32 bgColor = m_IsTableFocused ? IM_COL32(30, 120, 255, 200) : IM_COL32(64.0f, 87.0f, 111.0f, 255.0f);
		
				ImGui::TableSetColumnIndex(column);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(bgColor));
			}
		}
		else if (isHoveringRow)
		{
			for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
			{
				ImGui::TableSetColumnIndex(column);
		
				constexpr ImVec4 hoverColor = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(hoverColor));
			}
		}
		else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity entityToTestForDescendancy)
			{
				return m_pScene->EntityIsDescendant(currentEntity, entityToTestForDescendancy);
			}))
		{
			constexpr ImU32 bgColor = IM_COL32(44.0f, 50.0f, 58.0f, 255.0f);
			for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
			{
				ImGui::TableSetColumnIndex(column);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(bgColor));
			}
		}
		
		if (isHoveringRow)
			m_HoveredEntity = currentEntity;
		
		EntityManager& entityManager = m_pScene->GetEntityManager();
		
		const bool isHiddenInGame = entityManager.Has<HiddenInGameComponent>(currentEntity);
		
		ImGui::TableSetColumnIndex(0);
		
		const bool hoversColumn0 = ImGui::IsMouseHoveringRect(cellRects[0].Min, cellRects[0].Max);
		
		const uint32_t nrOfAncestors = m_pScene->GetAllEntityAncestors(currentEntity).size();
		
		const float indentation = ImGui::GetTreeNodeToLabelSpacing() * (nrOfAncestors + 1);
		ImGui::Unindent(indentation);
		if (isHoveringRow || isHiddenInGame || entityIsSelected)
		{
			constexpr ImVec2 imageSize = ImVec2(22, 12);
			constexpr float imagePositionPadding = 3.0f;
			const ImVec4 imageTint = (hoversColumn0 || entityIsSelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
		
			const float availableWidth = ImGui::GetContentRegionAvail().x;
			const float availableHeight = ImGui::GetTextLineHeightWithSpacing();
			const float offsetX = (availableWidth - imageSize.x) * 0.5f;
			const float offsetY = (availableHeight - imageSize.y) * 0.5f;
		
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX + imagePositionPadding);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY - imagePositionPadding);
		
			const std::shared_ptr<Texture2D> visibilityIconTexture = isHiddenInGame ? AssetManager::Get<Texture2D>(m_HideEntityTextureIconHandle) : AssetManager::Get<Texture2D>(m_ShowEntityTextureIconHandle);
			ImGui::Image((ImTextureID)visibilityIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, imageSize, ImVec2(0, 0), ImVec2(1, 1), imageTint);
		
			if (hoversColumn0)
				UI::Utility::DrawTooltip("Toggles the visibility of this entity in the level editor.");
		
			if (hoversColumn0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				if (isHiddenInGame)
					entityManager.Remove<HiddenInGameComponent>(currentEntity);
				else
					entityManager.Add<HiddenInGameComponent>(currentEntity);
			}
		}
		ImGui::Indent(indentation);
		
		ImGui::TableSetColumnIndex(1);
		
		auto& nameComponent = entityManager.Get<NameComponent>(currentEntity);
		
		ImGuiStyle& style = ImGui::GetStyle();
		const ImVec4 originalHoverColor = style.Colors[ImGuiCol_HeaderHovered];
		const ImVec4 originalActiveColor = style.Colors[ImGuiCol_HeaderActive];
		
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		
		const bool isParent = entityManager.Has<ParentComponent>(currentEntity);
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth;
		if (isParent)
			nodeFlags |= (ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
		else
			nodeFlags |= ImGuiTreeNodeFlags_Leaf;
		
		ImGui::Indent(indentation);
		const bool isOpen = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)currentEntity, nodeFlags, nameComponent.Name.c_str());
		bool hoversName = ImGui::IsItemHovered();
		
		ImGui::Unindent(indentation);
		
		style.Colors[ImGuiCol_HeaderHovered] = originalHoverColor;
		style.Colors[ImGuiCol_HeaderActive] = originalActiveColor;
		
		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover))
		{
			m_DraggedEntity = currentEntity;
		
			ImGui::SetDragDropPayload("OUTLINER_TABLE_PAYLOAD", &currentEntity, sizeof(entity), ImGuiCond_::ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("OUTLINER_TABLE_PAYLOAD"))
			{
				const entity toBecomeChild = *(entity*)payLoad->Data;
		
				if (std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
					{
						return e != currentEntity && !m_pScene->EntityIsDescendant(e, currentEntity);
					}))
				{
					//We know ALL entities are not the one being dragged upon and we know the target for drag drop is not child of any dragged entity.
					//Now we either attach or detach.

					//Rule 1: if ALL selected entities are direct children of targetted entity, we should detach all of them outright:
					if (std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
						{
							return m_pScene->EntityIsChild(e, currentEntity);
						}))
					{
						std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e)
							{
								m_pScene->DetachEntity(e);
							});
					} //Rule 2: If any selected entity is direct child of target entity, ALL entities remain as or become direct children of targetted entity.
					else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
						{
							return m_pScene->EntityIsChild(e, currentEntity);
						}))
					{
						std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
							{
								if (m_pScene->EntityIsChild(e, currentEntity))
								return;

						m_pScene->AttachEntity(e, currentEntity);
							});
					}
					else //Just regular attach all:
					{
						std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, currentEntity](entity e)
							{
								m_pScene->AttachEntity(e, currentEntity);
							});
					}
				}
			}
			ImGui::EndDragDropTarget();
		}
		
		const bool hoversColumn1 = ImGui::IsMouseHoveringRect(cellRects[1].Min, cellRects[1].Max);
		
		ImRect collapseArrowRect;
		collapseArrowRect.Min = cellRects[1].Min;
		collapseArrowRect.Min.x += indentation;
		collapseArrowRect.Min.x += ImGui::GetStyle().FramePadding.x;
		collapseArrowRect.Min.x += 6.0f;
		collapseArrowRect.Max = ImVec2(collapseArrowRect.Min.x, cellRects[1].Max.y);
		collapseArrowRect.Max.x += ImGui::GetTreeNodeToLabelSpacing();
		collapseArrowRect.Max.x -= 9.0f;
		const bool hoversCollapseArrowRect = ImGui::IsMouseHoveringRect(collapseArrowRect.Min, collapseArrowRect.Max);
		ImGui::GetWindowDrawList()->AddRect(collapseArrowRect.Min, collapseArrowRect.Max, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
		
		ImGui::TableSetColumnIndex(2);
		
		ImGui::Text("Entity");
		const bool hoversColumn2 = ImGui::IsMouseHoveringRect(cellRects[2].Min, cellRects[2].Max);
		
		const bool isHoveringSelectableRowSpace = isParent ? (hoversColumn1 && !hoversCollapseArrowRect) : hoversColumn1;
		
		if ((isHoveringSelectableRowSpace || hoversColumn2) && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
			OnMouseReleasedOverTableEntry();
		else if ((isHoveringSelectableRowSpace || hoversColumn2) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			OnTableEntryClicked();
		
		if (isOpen && entityManager.Has<ParentComponent>(currentEntity))
		{
			auto& pc = entityManager.Get<ParentComponent>(currentEntity);
			for (entity child : pc.Children)
				isHoveringRow |= DrawTableEntry(child);
		}
		
		if (isOpen)
			ImGui::TreePop();

		return isHoveringRow;
	}

	bool SceneHierarchyPanel::DrawTableRootEntry(bool& outIsOpen) noexcept
	{
		ImGui::TableNextRow();
		
		bool isHoveringRow = false;
		ImRect cellRects[3];
		
		for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
		{
			ImGui::TableSetColumnIndex(column);
		
			cellRects[column] = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
			cellRects[column].Max.y += ImGui::TableGetHeaderRowHeight() - 6.0f;
		
			isHoveringRow |= ImGui::IsMouseHoveringRect(cellRects[column].Min, cellRects[column].Max);
		}
		
		if (m_SceneTableEntrySelected)
		{
			const ImU32 bgColor = m_IsTableFocused ? IM_COL32(30, 120, 255, 200) : IM_COL32(64.0f, 87.0f, 111.0f, 255.0f);
			for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
			{
				ImGui::TableSetColumnIndex(column);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(bgColor));
			}
		}
		else if (isHoveringRow)
		{
			constexpr ImVec4 hoverColor = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
			for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
			{
				ImGui::TableSetColumnIndex(column);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(hoverColor));
			}
		}
		else if (!m_SelectedEntities.empty())
		{
			constexpr ImU32 bgColor = IM_COL32(44.0f, 50.0f, 58.0f, 255.0f);
			for (int column = 0; column < SceneHierarchyPanel_private::TABLE_COLUMN_COUNT; column++)
			{
				ImGui::TableSetColumnIndex(column);
				ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(bgColor));
			}
		}
		
		if (isHoveringRow)
			m_SceneTableEntryIsHovered = true;
		else
			m_SceneTableEntryIsHovered = false;
		
		ImGui::TableSetColumnIndex(0);
		
		const bool hoversColumn0 = ImGui::IsMouseHoveringRect(cellRects[0].Min, cellRects[0].Max);
		
		if (isHoveringRow || m_SceneIsHiddenInGame || m_SceneTableEntrySelected)
		{
			constexpr ImVec2 imageSize = ImVec2(22, 12);
			constexpr float imagePositionPadding = 3.0f;
			const ImVec4 imageTint = (hoversColumn0 || m_SceneTableEntrySelected) ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
		
			const float availableWidth = ImGui::GetContentRegionAvail().x;
			const float availableHeight = ImGui::GetTextLineHeightWithSpacing();
			const float offsetX = (availableWidth - imageSize.x) * 0.5f;
			const float offsetY = (availableHeight - imageSize.y) * 0.5f;
		
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX + imagePositionPadding);
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY - imagePositionPadding);
		
			const std::shared_ptr<Texture2D> visibilityIconTexture = m_SceneIsHiddenInGame ? AssetManager::Get<Texture2D>(m_HideEntityTextureIconHandle) : AssetManager::Get<Texture2D>(m_ShowEntityTextureIconHandle);
			ImGui::Image((ImTextureID)visibilityIconTexture->GetSRVDescriptorHandle().GPUHandle.ptr, imageSize, ImVec2(0, 0), ImVec2(1, 1), imageTint);
		
			if (hoversColumn0)
				UI::Utility::DrawTooltip("Toggles the visibility of this entity in the level editor.");
		
			if (hoversColumn0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				m_SceneIsHiddenInGame = !m_SceneIsHiddenInGame;
				EntityManager& entityManager = m_pScene->GetEntityManager();

				entityManager.Collect<IDComponent>().Do([this, &entityManager](entity e)
					{
						const bool isHidden = entityManager.Has<HiddenInGameComponent>(e);
						if (m_SceneIsHiddenInGame && !isHidden)
							entityManager.Add<HiddenInGameComponent>(e);
						else if (!m_SceneIsHiddenInGame && isHidden)
							entityManager.Remove<HiddenInGameComponent>(e);
					});
			}
		}
		
		ImGui::TableSetColumnIndex(1);
		
		ImGuiStyle& style = ImGui::GetStyle();
		const ImVec4 originalHoverColor = style.Colors[ImGuiCol_HeaderHovered];
		const ImVec4 originalActiveColor = style.Colors[ImGuiCol_HeaderActive];
		
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
		
		const bool isOpen = ImGui::TreeNodeEx(m_pScene->GetName().c_str(), ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
		
		style.Colors[ImGuiCol_HeaderHovered] = originalHoverColor;
		style.Colors[ImGuiCol_HeaderActive] = originalActiveColor;
		
		const bool hoversColumn1 = ImGui::IsMouseHoveringRect(cellRects[1].Min, cellRects[1].Max);
		
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("OUTLINER_TABLE_PAYLOAD"))
			{
				const entity entityToDetach = *(entity*)payLoad->Data;
		
				EntityManager& entityManager = m_pScene->GetEntityManager();
		
				if (!std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager](entity e)
					{
						return entityManager.Has<RootComponent>(e);
					}))
				{
					std::for_each(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e1)
						{
							if (!std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this, e1](entity e2)
								{
									return m_pScene->EntityIsParent(e1, e2);
								}))
							{
								m_pScene->DetachEntity(e1);
							}
						});
				}
			}
			ImGui::EndDragDropTarget();
		}
		
		ImRect collapseArrowRect;
		collapseArrowRect.Min = cellRects[1].Min;
		collapseArrowRect.Min.x += ImGui::GetStyle().ItemSpacing.x;
		collapseArrowRect.Max = ImVec2(collapseArrowRect.Min.x, cellRects[1].Max.y);
		collapseArrowRect.Max.x += ImGui::GetFontSize();
		collapseArrowRect.Max.x += 3.0f;
		const bool hoversCollapseArrowRect = ImGui::IsMouseHoveringRect(collapseArrowRect.Min, collapseArrowRect.Max);
		ImGui::GetWindowDrawList()->AddRect(collapseArrowRect.Min, collapseArrowRect.Max, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)));
		
		ImGui::TableSetColumnIndex(2);
		
		ImGui::Text("Scene");
		const bool hoversColumn2 = ImGui::IsMouseHoveringRect(cellRects[2].Min, cellRects[2].Max);
		
		if (((hoversColumn1 && !hoversCollapseArrowRect) || hoversColumn2) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
		{
			if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
				m_SceneTableEntrySelected = !m_SceneTableEntrySelected;
			else
			{
				m_SceneTableEntrySelected = true;
				m_SelectedEntities.clear();
			}
		}
		
		outIsOpen = isOpen;

		return isHoveringRow;
	}

	void SceneHierarchyPanel::DrawDraggingTooltip() noexcept
	{
		if (!ImGui::IsDragDropActive())
			return;
		
		if (!m_pScene)
			return;
		
		const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("OUTLINER_TABLE_PAYLOAD", ImGuiDragDropFlags_AcceptPeekOnly);
		if (!payload)
			return;
		
		const entity toBecomeChild = *(entity*)payload->Data;
		EntityManager& entityManager = m_pScene->GetEntityManager();
		
		if (m_HoveredEntity != NULL_ENTITY)
		{
			auto& nameComponent = entityManager.Get<NameComponent>(m_HoveredEntity);
		
			if (std::all_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager, this](entity e)
				{
					return entityManager.Has<IsChildComponent>(e) && (entityManager.Get<IsChildComponent>(e).Parent == m_HoveredEntity); //Entity is parent?
				}))
			{
				ImGui::SetTooltip("Detach from %s.", nameComponent.Name.c_str());
			}
			else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e)
				{
					return e == m_HoveredEntity;
				}))
			{
				ImGui::SetTooltip("Cannot attach entity to self.", nameComponent.Name.c_str());
			}
			else if (std::any_of(m_SelectedEntities.begin(), m_SelectedEntities.end(), [this](entity e)
				{
					return m_pScene->EntityIsDescendant(e, m_HoveredEntity);
				}))
			{
				ImGui::SetTooltip("Parent entity cannot become the child of their descendant.");
			}
			else
				ImGui::SetTooltip("Attach to %s.", nameComponent.Name.c_str());
		}
		else
		{
			if (m_SceneTableEntryIsHovered || m_TableEmptySpaceHovered)
			{
				if (auto it = std::find_if(m_SelectedEntities.begin(), m_SelectedEntities.end(), [&entityManager](entity e)
					{
						return entityManager.Has<RootComponent>(e);
					}); it != m_SelectedEntities.end())
				{
					ImGui::SetTooltip("%s is already attached to root.", entityManager.Get<NameComponent>(*it).Name.c_str());
				}
				else
					ImGui::SetTooltip("Move to root.");
			}
			else
			{
				std::string tooltip = entityManager.Get<NameComponent>(m_DraggedEntity).Name;
				if (m_SelectedEntities.size() > 1)
					tooltip += std::format(" and {} other{}.", m_SelectedEntities.size() - 1, (m_SelectedEntities.size() - 1) > 1 ? "s" : "");
		
				ImGui::SetTooltip(tooltip.c_str());
			}
		}
	}	
}
