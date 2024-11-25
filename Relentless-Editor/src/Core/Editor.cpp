#include "Editor.h"

namespace Relentless
{
	void Editor::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::RawMouseMoveEvent:
		{
			if (m_HoveringSceneViewport)
			{
				if (Mouse::IsButtonPressed(RLS_BUTTON::Left) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
				{
					m_IsPanningMouse = true;

					Mouse::ConfineCursor(vMin.x, vMax.x, vMax.y, vMin.y);
					Mouse::HideCursor();
					event.StopPropagation();
				}
			}

			break;
		}
		case EventType::RightMouseButtonPressedEvent:
		case EventType::MiddleMouseButtonPressedEvent:
		{
			if (m_HoveringSceneViewport)
			{
				Mouse::ConfineCursor(vMin.x, vMax.x, vMax.y, vMin.y);
				Mouse::HideCursor();
				event.StopPropagation();
			}
			break;
		}
		case EventType::RightMouseButtonReleasedEvent:
		case EventType::MiddleMouseButtonReleasedEvent:
		{
			if (m_HoveringSceneViewport)
			{
				Mouse::FreeCursor();
				Mouse::ShowCursor();
				event.StopPropagation();
			}
			break;
		}
		case EventType::LeftMouseButtonReleasedEvent:
		{
			if (!m_HoveringSceneViewport)
				return;

			if (ImGuizmo::IsUsing())
				return;
			
			if (m_HoveringSceneViewport && m_IsPanningMouse)
			{
				Mouse::FreeCursor();
				Mouse::ShowCursor();
				event.StopPropagation();
				m_IsPanningMouse = false;
				return;
			}

			const bool lCtrlDown = Keyboard::IsKeyPressed(RLS_KEY::LCtrl);
			const bool lShiftDown = Keyboard::IsKeyPressed(RLS_KEY::LShift);
			const bool isHoveringEntity = m_HoveredEntity != NULL_ENTITY;

			if (!isHoveringEntity || (!lCtrlDown && !lShiftDown))
				m_Selection.DeselectAllEntities();

			if (isHoveringEntity)
			{
				if (lCtrlDown && m_Selection.IsEntitySelected(m_HoveredEntity))
					m_Selection.DeselectEntity(m_HoveredEntity);
				else
					m_Selection.SelectEntity(m_HoveredEntity);
			}

			break;
		}
		case EventType::KeyPressedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right);
			if (!isNavigatingScene)
			{
				const RLS_KEY key = EVENT(KeyPressedEvent).key;
				switch (key)
				{
				case RLS_KEY::Q: m_CurrentGizmoType = EGizmoType::None; break;
				case RLS_KEY::W: m_CurrentGizmoType = (EGizmoType)ImGuizmo::TRANSLATE; break;
				case RLS_KEY::E: m_CurrentGizmoType = (EGizmoType)ImGuizmo::ROTATE; break;
				case RLS_KEY::R: m_CurrentGizmoType = (EGizmoType)ImGuizmo::SCALE; break;
				case RLS_KEY::T: m_CurrentGizmoMode = (EGizmoMode)!(bool)m_CurrentGizmoMode; break;
				case RLS_KEY::I:
				{
					if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
						m_ImmersiveModeEnabled = !m_ImmersiveModeEnabled;

					break;
				}
				case RLS_KEY::A:
				{
					if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
					{
						m_pActiveScene->GetEntityManager().Collect<IDComponent>().Do([this](entity e)
							{
								if (!m_Selection.IsEntitySelected(e))
									m_Selection.SelectEntity(e);
							});
					}
					break;
				}
				case RLS_KEY::H:
				{
					if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
					{
						m_pActiveScene->GetEntityManager().Collect<HiddenInGameComponent>().Do([this](entity e)
							{
								m_pActiveScene->SetEntityVisibleInGame(e, true);
							});
					}
					else
					{
						const std::vector<entity>& selectedEntities = m_Selection.GetSelectedEntities();
						EntityManager& entityManager = m_pActiveScene->GetEntityManager();

						for (int i = selectedEntities.size() - 1; i >= 0; --i)
						{
							const entity currentEntity = selectedEntities[i];

							m_Selection.DeselectEntity(currentEntity);
								m_pActiveScene->SetEntityVisibleInGame(currentEntity, false);
						}
					}
					break;
				}
				case RLS_KEY::Delete:
				{
					const std::vector<entity>& selectedEntities = m_Selection.GetSelectedEntities();
					EntityManager& entityManager = m_pActiveScene->GetEntityManager();

					for (int i = selectedEntities.size() - 1; i >= 0; --i)
					{
						const entity currentEntity = selectedEntities[i];
						m_pActiveScene->DestroyEntity(currentEntity);
					}
					break;
				}
				}
			}
			event.StopPropagation();
			break;
		}
		}

		m_ContentBrowserPanel.OnEvent(event);
	}

	void Editor::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		UI_DrawMainMenuBar();

		ImGuiWindowClass viewportWindowClass;
		viewportWindowClass.DockNodeFlagsOverrideSet = m_ImmersiveModeEnabled ? ImGuiDockNodeFlags_NoTabBar : 0;
		ImGui::SetNextWindowClass(&viewportWindowClass);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Scene");

		//UI_DrawSceneStateIcons();

		if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
		{
			m_DisplayInspectorPanel = false;
		}

		vMin = ImGui::GetWindowContentRegionMin();
		vMax = ImGui::GetWindowContentRegionMax();
		vMin.x += ImGui::GetWindowPos().x;
		vMin.y += ImGui::GetWindowPos().y;
		vMax.x += ImGui::GetWindowPos().x;
		vMax.y += ImGui::GetWindowPos().y;

		m_HoveringSceneViewport = ImGui::IsWindowHovered();

		const float toolBarPadding = m_ImmersiveModeEnabled ? 0 : 24.0f;
		ImVec2 mousePosition = ImGui::GetMousePos();
		ImVec2 windowPosition = ImGui::GetWindowPos();
		ImVec2 mousePositionInSceneClientArea = ImVec2(mousePosition.x - windowPosition.x, mousePosition.y - windowPosition.y - toolBarPadding);
		m_pActiveScene->SetMousePosition(mousePositionInSceneClientArea);
		auto xx = ImGui::GetContentRegionAvail().x;
		auto yy = ImGui::GetContentRegionAvail().y;

		if (m_ViewportPanelSize.x != xx || m_ViewportPanelSize.y != yy)
		{
			//Verify values are valid (as is not the case when shutting down the program!)
			if (!(ImGui::GetContentRegionAvail().x < 0.0f) && !(ImGui::GetContentRegionAvail().y < 0.0f))
			{
				m_ViewportPanelSize.x = ImGui::GetContentRegionAvail().x;
				m_ViewportPanelSize.y = ImGui::GetContentRegionAvail().y;
				m_SceneViewportChanged = true;
			}
		}

		m_pSceneRenderer->OnImGuiRender(ImVec2(m_ViewportPanelSize.x, m_ViewportPanelSize.y));

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("MULTIPLE_ENTRIES_DRAG_DROP"))
			{
				const std::vector<std::string>& selectedEntries = m_ContentBrowserPanel.GetSelectedEntries();
				std::for_each(selectedEntries.begin(), selectedEntries.end(), [this](const std::string& path)
					{
						if (!AssetRegistry::IsFilepathMapped(path))
						return;

				const AssetHandle handle = AssetManager::GetHandleByPath(path);
				if (handle.Type != AssetType::Mesh)
					return;

				CreateEntityFromDroppedMesh(handle);
					});
			}
			ImGui::EndDragDropTarget();
		}

		if (m_CurrentGizmoType != EGizmoType::None)
			ManipulateTransformGizmo();

		ImGui::End();
		ImGui::PopStyleVar();

		UI_DrawStatisticsPanel();

		m_pOutlinerPanel->OnImGuiRender(m_DisplayOutlinerPanel && !m_ImmersiveModeEnabled);
		m_InspectorPanel.OnImGuiRender(m_DisplayInspectorPanel && !m_ImmersiveModeEnabled);
		m_SceneRendererPanel.OnImGuiRender(m_DisplaySceneRendererPanel && !m_ImmersiveModeEnabled);
		m_PropertiesPanel.OnImGuiRender(m_DisplayPropertiesPanel && !m_ImmersiveModeEnabled);
		m_ContentBrowserPanel.OnImGuiRender(m_DisplayContentBrowserPanel && !m_ImmersiveModeEnabled);
		m_MetricsPanel.OnImGuiRender(m_DisplayMetricsPanel && !m_ImmersiveModeEnabled);
	}

	void Editor::OnCreate() noexcept
	{
		m_pOutlinerPanel = std::make_unique<OutlinerPanel>(this);

		m_Selection.OnSelectionChanged.Connect(this, &Editor::OnEntitySelectionChanged);

		SetActiveScene(std::make_shared<Scene>());
		
		LoadStarterMeshes();
		CreateStartScene();

		m_pSceneRenderer = std::make_shared<SceneRenderer>(m_pActiveScene);
		m_pUtilityRenderer = std::make_shared<UtilityRenderer>();

		m_PropertiesPanel.SetOnMaterialSelectedCallback([this](const AssetHandle& materialHandle)
			{
				m_InspectorPanel.SetContext(materialHandle, InspectedAssetType::MATERIAL);
				m_DisplayInspectorPanel = true;
			});

		m_ContentBrowserPanel.SetOnAssetSelectedCallback([this](const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)
			{
				m_InspectorPanel.SetContext(AssetHandle, inspectedAssetType);
				if (inspectedAssetType == InspectedAssetType::NONE)
					m_DisplayInspectorPanel = false;
				else
					m_DisplayInspectorPanel = true;
			});

		//m_PlayButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\PlayButton.png"), "");
		//m_StopButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\StopButton.png"), "");
		//m_PauseButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\PauseButton.png"), "");
		//m_SimulateButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\SimulateButton.png"), "");
		//m_StepButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\StepButton.png"), "");

		const std::filesystem::path srcPath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures/puresky.rasset");

		AssetHandle handle;
		if (AssetManager::RequestLoadAsset(srcPath, handle))
		{
			std::shared_ptr<Texture2D> pTexture = AssetManager::Get<Texture2D>(handle);
			m_pUtilityRenderer->ConvertEquirectangularToCubeMap(pTexture, [this](std::shared_ptr<TextureCube> pTextureCube)
				{
					m_pActiveScene->m_pSkyBox = pTextureCube;
			m_pUtilityRenderer->CreateIrradianceMap(pTextureCube, [this, pTextureCube](std::shared_ptr<TextureCube> pIrradianceMap)
				{
					m_pActiveScene->m_pIrradianceMap = pIrradianceMap;
				});
			m_pUtilityRenderer->CreateRadianceMap(pTextureCube, [this, pTextureCube](std::shared_ptr<TextureCube> pRadianceMap)
				{
					m_pActiveScene->m_pRadianceMap = pRadianceMap;
				});
				});
		}

		

		m_PropertiesPanel.SetActiveScene(m_pActiveScene.get());
		m_SceneRendererPanel.SetActiveRenderer(m_pSceneRenderer);

		SetActiveScene(m_pActiveScene);
	}

	void Editor::OnDestroy() noexcept
	{
		m_Selection.OnSelectionChanged.Detach(this);
	}

	void Editor::OnUpdate(const float deltaTime) noexcept
	{
		PROFILE_FUNC;

		if (m_SceneViewportChanged)
			OnSceneViewportChanged();

		m_pActiveScene->GetEditorCamera()->Update();
		m_pActiveScene->OnUpdate(deltaTime);
	}

	void Editor::OnRender() noexcept
	{
		PROFILE_FUNC;

		m_pSceneRenderer->Begin();
		m_pSceneRenderer->IssueRenderPasses();
		m_pSceneRenderer->End();
	}

	//This is post issuing the render commands.
	//Important: That does NOT mean it is finished.
	//To be specific: It should be assumed it is not
	void Editor::OnPostRender() noexcept
	{
		PROFILE_FUNC;

		if (m_HoveringSceneViewport)
		{
			m_HoveredEntity = m_pActiveScene->GetHoveredEntity();
		}
		else
			m_HoveredEntity = NULL_ENTITY;

		m_SceneRendererPanel.OnPostRender();
	}

	Selection& Editor::GetSelection() noexcept
	{
		return m_Selection;
	}

	void Editor::SetActiveScene(const std::shared_ptr<Scene>& pScene) noexcept
	{
		m_Selection.DeselectAllEntities();

		if (m_pActiveScene)
			m_pActiveScene->OnEntityPreDestroyed.Detach(this);

		m_pActiveScene = pScene;
		m_pEditorScene = m_pActiveScene;

		m_pActiveScene->SetViewportPanelSize(m_ViewportPanelSize);

		m_pActiveScene->OnEntityPreDestroyed.Connect(this, &Editor::OnEntityPreDestroyed);

		OnSceneChanged(m_pActiveScene.get());
	}

	void Editor::LoadStarterMeshes() noexcept
	{
		const std::array<std::string, 11> starterMeshes
		{
			"Cube.rasset",
			"Capsule.rasset",
			"Cone.rasset",
			"Cylinder.rasset",
			"Icosphere.rasset",
			"Plane.rasset",
			"Quad.rasset",
			"Sphere.rasset",
			"Torus.rasset",
			"Triangle.rasset",
			"UtahTeapot.rasset"
		};

		const std::string meshPath = std::string(ENGINE_ASSET_DIRECTORY) + std::string("Models\\StarterContent\\");

		MeshImportSettings importSettings = {};
		importSettings.ImportMaterialsAndTextures = false;
		for (auto& mesh : starterMeshes)
		{
			const std::string fullMeshPath(meshPath + std::string(mesh));
			AssetHandle throwAway = NULL_HANDLE;
			AssetManager::RequestLoadAsset(fullMeshPath, throwAway);
		}
	}

	void Editor::CreateStartScene() noexcept
	{
		entity parent = m_pActiveScene->CreateShape(Shape::Cube);
		entity child = m_pActiveScene->CreateShape(Shape::Torus);
		entity otherCube = m_pActiveScene->CreateShape(Shape::Capsule);
		m_pActiveScene->SetWorldLocation(parent, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pActiveScene->SetWorldLocation(child, DirectX::XMFLOAT3(5.0f, 0.0f, 0.0f));
		m_pActiveScene->SetWorldLocation(otherCube, DirectX::XMFLOAT3(-5.0f, 0.0f, 0.0f));
	}

	void Editor::OnSceneViewportChanged() noexcept
	{
		m_ViewportPanelSize.x = std::max(1.0f, m_ViewportPanelSize.x);
		m_ViewportPanelSize.y = std::max(1.0f, m_ViewportPanelSize.y);
		m_pActiveScene->SetViewportPanelSize(m_ViewportPanelSize);

		m_pSceneRenderer->OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_SceneViewportChanged = false;
	}

	void Editor::ManipulateTransformGizmo() noexcept
	{
		const std::vector<entity>& selectedEntities = m_Selection.GetSelectedEntities();
		if (selectedEntities.empty())
			return;

		std::vector<entity> participatingEntities;
		participatingEntities.reserve(selectedEntities.size());

		for (int i = 0; i < selectedEntities.size(); ++i)
		{
			const entity e = selectedEntities[i];

			if (!std::any_of(selectedEntities.begin(), selectedEntities.end(), [&](entity currentEntity)
				{
					return m_pActiveScene->EntityIsAncestor(currentEntity, e);
				}))
			{
				participatingEntities.push_back(e);
			}
		}
		
		if (participatingEntities.empty())
			return;

		const entity pivotEntity = participatingEntities.back();

		Matrix pivot = m_CurrentGizmoMode == EGizmoMode::World ? m_pActiveScene->GetWorldTransform(pivotEntity) : m_pActiveScene->GetLocalTransform(pivotEntity);
		const Matrix pivotNonManipulated = pivot;

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		const ImGuizmo::MODE mode = (m_CurrentGizmoType == EGizmoType::Scale) ? ImGuizmo::LOCAL : (ImGuizmo::MODE)m_CurrentGizmoMode;

		const bool manipulated = ImGuizmo::Manipulate(*m_pActiveScene->GetEditorCamera()->GetViewTransform().View.m, *m_pActiveScene->GetEditorCamera()->GetViewTransform().Projection.m, (ImGuizmo::OPERATION)m_CurrentGizmoType, mode, pivot.m[0]);
		if (manipulated)
		{
			m_pActiveScene->SetWorldTransform(pivotEntity, pivot);

			const Matrix pivotInverseMatrix = pivotNonManipulated.Invert();

			participatingEntities.pop_back();

			for (auto e : participatingEntities)
			{
				const Matrix entityWorld = m_pActiveScene->GetWorldTransform(e);
				const Matrix entityLocalToPivotMatrix = entityWorld * pivotInverseMatrix;
				const Matrix newEntityMatrix = entityLocalToPivotMatrix * pivot;

				m_pActiveScene->SetWorldTransform(e, newEntityMatrix);
			}
		}
	}

	void Editor::SetSceneContext(std::shared_ptr<Scene> pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is invalid.");

		m_pSceneRenderer->SetContext(pScene);

		m_PropertiesPanel.SetActiveScene(pScene.get());

		pScene->GetEditorCamera()->SetViewport(FloatRect(0, 0, m_ViewportPanelSize.x, m_ViewportPanelSize.y));
	}

	void Editor::UI_DrawStatisticsPanel() noexcept
	{
		if (!m_DisplayStatisticsPanel || m_ImmersiveModeEnabled)
			return;

		ImGui::Begin("Stats");
		ImGui::Text("Hovered entity:");
		ImGui::SameLine();

		if (m_HoveredEntity != NULL_ENTITY && m_pActiveScene->GetEntityManager().Exists(m_HoveredEntity))
			ImGui::Text("%s (%d)", m_pActiveScene->GetEntityManager().Get<NameComponent>(m_HoveredEntity).Name.c_str(), (m_HoveredEntity >> 12));
		else
			ImGui::Text("None");

		ImGui::Text("#Shader bindable descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", Application::Get().GetMemorymanager().GetShaderBindableDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#CBV/SRV/UAV descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", Application::Get().GetMemorymanager().GetCBVSRVUAVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#RTV descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", Application::Get().GetMemorymanager().GetRTVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#DSV descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", Application::Get().GetMemorymanager().GetDSVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#Constant buffer sets: TODO!!");

		ImGui::End();
	}

	void Editor::UI_DrawMainMenuBar() noexcept
	{
		if (m_ImmersiveModeEnabled)
			return;

		if (!ImGui::BeginMainMenuBar())
			return;

		if (ImGui::BeginMenu("File"))
		{
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Scene Hierarchy Panel", nullptr, &m_DisplayOutlinerPanel);
			ImGui::MenuItem("Content Browser Panel", nullptr, &m_DisplayContentBrowserPanel);
			ImGui::MenuItem("Properties Panel", nullptr, &m_DisplayPropertiesPanel);
			ImGui::MenuItem("Inspector Panel", nullptr, &m_DisplayInspectorPanel);
			ImGui::MenuItem("Metrics Panel", nullptr, &m_DisplayMetricsPanel);
			ImGui::MenuItem("Scene Renderer Panel", nullptr, &m_DisplaySceneRendererPanel);
			ImGui::MenuItem("Statistics Panel", nullptr, &m_DisplayStatisticsPanel);

			ImGui::MenuItem("Immersive Mode", "Ctrl + i", &m_ImmersiveModeEnabled);
			if (ImGui::MenuItem("Full Screen", "Alt + Enter", Window::IsFullScreen()))
			{
				Window::PrepareForFullScreenToggling();
			}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	void Editor::CreateEntityFromDroppedMesh(const AssetHandle& meshHandle) noexcept
	{
		std::shared_ptr<Mesh> pMesh = AssetManager::Get<Mesh>(meshHandle);
		const entity newEntity = m_pActiveScene->CreateEntity(pMesh->GetName().c_str());

		EntityManager& entityManager = m_pActiveScene->GetEntityManager();
		MeshRendererComponent& mrc = entityManager.Add<MeshRendererComponent>(newEntity);
		mrc.AssetHandle = AssetManager::GetDefaultMaterialHandle();
		
		MeshFilterComponent& mfc = entityManager.Add<MeshFilterComponent>(newEntity);
		mfc.AssetHandle = meshHandle;
		
		const Transform& transform = pMesh->GetOffsetTransform();
		
		auto& tc = entityManager.Get<TransformComponent>(newEntity);
		m_pActiveScene->SetWorldTransform(newEntity, transform.Matrix);
	}

	void Editor::OnEntitySelectionChanged(entity e, ESelectionState selectionState)
	{
		if (selectionState == ESelectionState::Selected)
			m_pActiveScene->GetEntityManager().AddOrReplace<SelectedInEditorComponent>(e);
		else
			m_pActiveScene->GetEntityManager().Remove<SelectedInEditorComponent>(e);
	}

	void Editor::OnEntityPreDestroyed(entity e) noexcept
	{
		if (m_Selection.IsEntitySelected(e))
			m_Selection.DeselectEntity(e);
	}

}
