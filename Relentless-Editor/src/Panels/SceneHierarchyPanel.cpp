#include "SceneHierarchyPanel.h"
#include "../Core/EditorLayer.h"
namespace Relentless
{
	SceneHierarchyPanel::SceneHierarchyPanel() noexcept
		: m_pScene{ nullptr },
		  m_SelectedEntity{ NULL_ENTITY },
		  m_EntityScheduledForDestruction{ NULL_ENTITY }
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
				if (m_IsTableFocused && Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
					OnSelectAll();
				break;
			}
			}
		}
		}
	}

	void SceneHierarchyPanel::OnImGuiRender(const bool show) noexcept
	{
		m_HoveredEntity = NULL_ENTITY;

		if (!show)
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
		ImGui::Begin("Outliner");
		ImGui::PopStyleVar();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);
		UI::SearchBar("OutlinerSearchBar", "Search...", true);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0f);

		constexpr ImVec4 evenTableRowBgColor = ImVec4(21.0f / 255.0f, 21.0f / 255.0f, 21.0f / 255.0f, 255.0f / 255.0f);
		constexpr ImVec4 oddTableRowBgColor = ImVec4(26.0f / 255.0f, 26.0f / 255.0f, 26.0f / 255.0f, 255.0f / 255.0f);

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBg, evenTableRowBgColor);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBgAlt, oddTableRowBgColor);

		constexpr ImGuiTableFlags flags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody;
		
		constexpr int COLUMN_COUNT = 3;

		if (ImGui::BeginTable("OutlinerTable", COLUMN_COUNT, flags))
		{
			m_IsTableFocused = ImGui::IsWindowFocused();

			ImGui::TableSetupScrollFreeze(0, 1);

			ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize, 30.0f);
			ImGui::TableSetupColumn("Item Label", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
			ImGui::TableHeadersRow();

			for (int column = 0; column < COLUMN_COUNT; column++)
			{
				ImGui::TableSetColumnIndex(column);
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

			m_pScene->GetEntityManager().Collect<NameComponent>().Do([&](entity currentEntity, const NameComponent& nameComponent)
				{
					const bool entityIsSelected = IsEntitySelected(currentEntity);

					ImGui::TableNextRow();

					bool isHoveringRow = false;
					constexpr ImVec4 hoverColor = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
					ImRect cellRects[3];
					for (int column = 0; column < COLUMN_COUNT; column++)
					{
						ImGui::TableSetColumnIndex(column);
						cellRects[column] = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
						cellRects[column].Max.y += ImGui::TableGetHeaderRowHeight() - 6.0f;
						
						isHoveringRow |= ImGui::IsMouseHoveringRect(cellRects[column].Min, cellRects[column].Max);
					}

					if (entityIsSelected)
					{
						for (int column = 0; column < COLUMN_COUNT; column++)
						{
							const ImU32 bgColor = m_IsTableFocused ? IM_COL32(30, 120, 255, 200) : IM_COL32(64.0f, 87.0f, 111.0f, 255.0f);

							ImGui::TableSetColumnIndex(column);
							ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(bgColor));
						}
					}
					else if (isHoveringRow)
					{
						for (int column = 0; column < COLUMN_COUNT; column++)
						{
							ImGui::TableSetColumnIndex(column);
							ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(hoverColor));
						}
					}

					if (isHoveringRow)
						m_HoveredEntity = currentEntity;

					const bool isHiddenInGame = m_pScene->GetEntityManager().Has<HiddenInGameComponent>(currentEntity);
					
					bool hoversColumn1 = ImGui::IsMouseHoveringRect(cellRects[1].Min, cellRects[1].Max);
					bool hoversColumn2 = ImGui::IsMouseHoveringRect(cellRects[2].Min, cellRects[2].Max);

					bool isHoveringVisibilityImage = false;
					for (int column = 0; column < COLUMN_COUNT; column++)
					{
						ImGui::TableSetColumnIndex(column);

						if (column == 0)
						{
							const bool hoversColumn0 = ImGui::IsMouseHoveringRect(cellRects[0].Min, cellRects[0].Max);

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
								isHoveringVisibilityImage = ImGui::IsItemHovered();

								if (hoversColumn0)
									UI::Utility::DrawTooltip("Toggles the visibility of this entity in the level editor.");

								if (hoversColumn0 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
								{
									if (isHiddenInGame)
										m_pScene->GetEntityManager().Remove<HiddenInGameComponent>(currentEntity);
									else 
										m_pScene->GetEntityManager().Add<HiddenInGameComponent>(currentEntity);
								}
							}
						}
						else if (column == 1)
						{
							ImGui::Text(nameComponent.Name.c_str());
							hoversColumn1 = ImGui::IsMouseHoveringRect(cellRects[1].Min, cellRects[1].Max);
						}
						else if (column == 2)
						{
							ImGui::Text("Entity");
							hoversColumn2 = ImGui::IsMouseHoveringRect(cellRects[2].Min, cellRects[2].Max);
						}
					}

					if ((hoversColumn1 || hoversColumn2) && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
						OnTableEntryClicked();
				});

			ImGui::EndTable();
		}

		ImGui::PopStyleColor(2);
		ImGui::End();
		return;


		PROFILE_FUNC;

		RLS_ASSERT(m_pScene, "Scene is nullptr.");

		ImGui::Begin("Scene Hierarchy");

		ImGuiTreeNodeFlags sceneNodeflags = ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_Selected | ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_DefaultOpen;
		sceneNodeflags |= ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_Framed;

		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[OPENSANS_BOLD_18];
		ImGui::PushFont(boldFont);
		bool opened = ImGui::TreeNodeEx(m_pScene->GetName().c_str(), sceneNodeflags, m_pScene->GetName().c_str());
		ImGui::PopFont();

		if (opened)
		{
			ImGui::Separator();

			m_pScene->GetEntityManager().Collect<NameComponent>().Do([this](entity e, NameComponent&)
				{
					if (m_pScene->GetEntityManager().Has<IsChildComponent>(e))
						return;

					DrawEntityNode(e);
				});
			ImGui::TreePop();
		}

		if (ImGui::BeginPopupContextWindow(0, 1, false /*over items*/))
		{
			bool createdNewEntity = false;

			if (ImGui::MenuItem("Create Empty"))
			{
				createdNewEntity = true;
				m_SelectedEntity = m_pScene->CreateEntity("Entity");
			}

			if (ImGui::BeginMenu("Shapes"))
			{
				if (ImGui::MenuItem("Triangle"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Triangle);
				}
				if (ImGui::MenuItem("Cube"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Cube);
				}
				if (ImGui::MenuItem("Cylinder"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Cylinder);
				}
				if (ImGui::MenuItem("Capsule"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Capsule);
				}
				if (ImGui::MenuItem("Cone"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Cone);
				}
				if (ImGui::MenuItem("Sphere"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Sphere);
				}
				if (ImGui::MenuItem("IcoSphere"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::IcoSphere);
				}
				if (ImGui::MenuItem("Torus"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Torus);
				}
				if ( ImGui::MenuItem("Quad"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Quad);
				}
				if (ImGui::MenuItem("Plane"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateShape(Shape::Plane);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Extras"))
			{
				if (ImGui::MenuItem("Utah Teapot"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateExtra(Extra::UtahTeapot);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Light"))
			{
				if (ImGui::MenuItem("Directional Light"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateLight("Directional Light", LightType::Directional);
				}
				if (ImGui::MenuItem("Point Light"))
				{
					createdNewEntity = true;
					m_SelectedEntity = m_pScene->CreateLight("Point Light", LightType::Point);
				}
				ImGui::EndMenu();
			}

			if (ImGui::MenuItem("Camera"))
			{
				createdNewEntity = true;
				m_SelectedEntity = m_pScene->CreateCamera("Camera");
			}

			if (createdNewEntity)
			{
				m_OnEntityCreatedCallBack(m_SelectedEntity);
			}

			ImGui::EndPopup();
		}

		//One can drop an entity onto the panel itself to return it to an unparented root state:
		if (ImGui::IsDragDropActive())
		{
			auto panelSize = ImGui::GetWindowSize();
			auto currentCursorPosition = ImGui::GetCursorPos();

			panelSize.x -= 20;
			panelSize.y -= (currentCursorPosition.y + 10);
			ImGui::InvisibleButton("SCENE_HIERARCHY_PANEL_EMPTY_SPACE", panelSize);
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("CHILD_PAYLOAD"))
				{
					auto& mgr = m_pScene->GetEntityManager();

					entity entityToOrphan = *(entity*)payLoad->Data;
					if (mgr.Has<IsChildComponent>(entityToOrphan))
					{
						entity parent = mgr.Get<IsChildComponent>(entityToOrphan).Parent;
						auto& children = mgr.Get<ParentComponent>(parent).Children;
						for (uint32_t childIndex{ 0u }; childIndex < children.size(); ++childIndex)
						{
							if (children[childIndex] == entityToOrphan)
							{
								children.erase(children.begin() + childIndex);
								if (children.empty())
									mgr.Remove<ParentComponent>(parent);

								break;
							}
						}
						mgr.Remove<IsChildComponent>(entityToOrphan);
					}
					mgr.AddOrReplace<RootComponent>(entityToOrphan);
				}
				ImGui::EndDragDropTarget();
			}
		}
		ImGui::End();

		//Any deletion of an entity in the scene hierarchy has been deferred until now:
		if (m_EntityScheduledForDestruction != NULL_ENTITY)
		{
			//We need to check if a child of the deleted entity is currently selected, and if so, notify the editor layer,
			//as that child will get destroyed in the process.
			if (m_pScene->GetEntityManager().Has<ParentComponent>(m_EntityScheduledForDestruction))
			{
				auto& children = m_pScene->GetEntityManager().Get<ParentComponent>(m_EntityScheduledForDestruction).Children;
				for (auto child : children)
				{
					if (child == m_SelectedEntity)
					{
						m_OnEntityDestroyedCallBack(child);
						m_SelectedEntity = NULL_ENTITY;
						break;
					}
				}
			}

			m_pScene->DestroyEntity(m_EntityScheduledForDestruction);
			m_OnEntityDestroyedCallBack(m_EntityScheduledForDestruction);
			if (m_SelectedEntity == m_EntityScheduledForDestruction)
			{
				m_SelectedEntity = NULL_ENTITY;
			}
			m_EntityScheduledForDestruction = NULL_ENTITY;
		}
	}

	void SceneHierarchyPanel::SetActiveScene(Scene* const pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is nullptr.");
		m_pScene = pScene;
		m_SelectedEntity = NULL_ENTITY;
	}

	void SceneHierarchyPanel::DrawEntityNode(const entity entityID) noexcept
	{
		auto& mgr = m_pScene->GetEntityManager();

		ImGuiTreeNodeFlags entityNodeflags = ((entityID == m_SelectedEntity) ? ImGuiTreeNodeFlags_Selected : 0);
		entityNodeflags |= mgr.Has<ParentComponent>(entityID) ? ImGuiTreeNodeFlags_DefaultOpen : ImGuiTreeNodeFlags_Leaf;
		entityNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_SpanFullWidth;
		entityNodeflags |= ImGuiTreeNodeFlags_::ImGuiTreeNodeFlags_OpenOnArrow;

		ImGuiStyle* style = &ImGui::GetStyle();
		style->Alpha = (entityID == m_SelectedEntity) ? 1.0f : 0.5f;

		auto& nc = mgr.Get<NameComponent>(entityID);

		bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entityID, entityNodeflags, nc.Name.c_str());
		if (ImGui::IsItemClicked())
		{
			m_SelectedEntity = entityID;
			m_OnEntitySelectedCallBack(m_SelectedEntity);
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::MenuItem("Delete Entity"))
			{
				m_EntityScheduledForDestruction = entityID;
			}

			ImGui::EndPopup();
		}

		if (ImGui::BeginDragDropSource())
		{
			ImGui::SetDragDropPayload("CHILD_PAYLOAD", &entityID, sizeof(entity), ImGuiCond_::ImGuiCond_Once);
			ImGui::EndDragDropSource();
		}
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("CHILD_PAYLOAD"))
			{
				entity toBecomeChild = *(entity*)payLoad->Data;
				if (entityID != toBecomeChild && !m_pScene->EntityIsDescendant(toBecomeChild, entityID))
				{
					m_pScene->ParentEntity(toBecomeChild, entityID);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (opened && mgr.Exists(entityID) && mgr.Has<ParentComponent>(entityID))
		{
			auto& pc = mgr.Get<ParentComponent>(entityID);
			for (auto child : pc.Children)
				DrawEntityNode(child);
		}

		if (opened)
			ImGui::TreePop();

		style->Alpha = 1.0f;
		
		
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
				m_SelectedEntities.clear();
				SelectEntity(m_HoveredEntity);
			}
		}
		else
		{
			if (!lCtrlPressed)
				m_SelectedEntities.clear();

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
	}
}