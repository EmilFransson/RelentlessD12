#include "EditorLayer.h"

namespace Relentless
{
	EditorLayer::EditorLayer(const std::string& layerName) noexcept
		: Layer{layerName}, 
		  m_ViewportPanelSize{100.0f, 100.0f}, 
		  m_SceneViewportChanged{ false },
		  m_HoveringSceneViewport{ false },
		  m_HoveredEntity{ NULL_ENTITY },
		  m_SelectedEntity{ NULL_ENTITY },
		  m_CurrentGizmoType{ GizmoType::NONE },
		  m_CurrentGizmoMode{ GizmoMode::LOCAL },
		  m_DisplaySceneHierarchyPanel{ true },
		  m_DisplayContentBrowserPanel{ true },
		  m_DisplayPropertiesPanel{ true },
		  m_DisplayInspectorPanel{false},
		  m_DisplayMetricsPanel{ true },
		  m_DisplaySceneRendererPanel{ true },
		  m_DisplayStatisticsPanel{ true },
		  m_ImmersiveModeEnabled{ false }
	{}

	void EditorLayer::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::RawMouseMoveEvent:
		{
			const bool isNavigatingScene = (m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right));
			if (isNavigatingScene)
				m_pActiveScene->GetEditorCamera()->OnMouseMove();
			event.StopPropagation();
			break;
		}
		case EventType::RightMouseButtonPressedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport;
			if (isNavigatingScene)
			{
				Mouse::ConfineCursor(vMin.x, vMax.x, vMax.y, vMin.y);
				Mouse::HideCursor();
				event.StopPropagation();
			}
			break;
		}
		case EventType::RightMouseButtonReleasedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport;
			if (isNavigatingScene)
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

			if (m_HoveredEntity == NULL_ENTITY)
			{
				if (m_SelectedEntity != NULL_ENTITY)
				{
					m_pActiveScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);
				}
				m_SelectedEntity = NULL_ENTITY;
				m_CurrentGizmoType = GizmoType::NONE;
			}
			else
			{
				if (m_SelectedEntity != NULL_ENTITY)
					m_pActiveScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);

				m_SelectedEntity = m_HoveredEntity;
				m_pActiveScene->GetEntityManager().Add<SelectedInEditorComponent>(m_SelectedEntity);
				m_CurrentGizmoType = GizmoType::TRANSLATE;
			}
			m_SceneHierarchyPanel.SetSelectedEntity(m_SelectedEntity);
			m_PropertiesPanel.SetSelectedEntity(m_SelectedEntity);

			break;
		}
		case EventType::KeyPressedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right);
			if (!isNavigatingScene)
			{
				RLS_KEY key = EVENT(KeyPressedEvent).key;
				if (key == RLS_KEY::Q)
					m_CurrentGizmoType = GizmoType::NONE;
				else if (key == RLS_KEY::W)
					m_CurrentGizmoType = (GizmoType)ImGuizmo::TRANSLATE;
				else if (key == RLS_KEY::E)
					m_CurrentGizmoType = (GizmoType)ImGuizmo::ROTATE;
				else if (key == RLS_KEY::R)
					m_CurrentGizmoType = (GizmoType)ImGuizmo::SCALE;
				else if (key == RLS_KEY::T)
					m_CurrentGizmoMode = (GizmoMode)!(bool)m_CurrentGizmoMode;
				else if (key == RLS_KEY::D && Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
					CopySelectedEntity();
				else if (key == RLS_KEY::Delete)
					DestroySelectedEntity();
				else if (key == RLS_KEY::N && Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
					m_CreateNewScene = true;
				else if (key == RLS_KEY::O && Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
				{
					//TODO: FIX IMOPORT
					RLS_ASSERT(false, "FIX!");
					
					//std::string filepath = FileDialogs::OpenFile("Relentless Scene (*.Relentless)\0*.Relentless\0");
					//if (!filepath.empty())
					//{
					//	m_Path = filepath;
					//}
					//ForceReset Keyboard!
				}
				else if (key == RLS_KEY::S && (Keyboard::IsKeyPressed(RLS_KEY::LCtrl) && Keyboard::IsKeyPressed(RLS_KEY::LShift)))
				{
					std::string filepath = FileDialogs::SaveFile("Relentless Scene (*.Relentless)\0*.Relentless\0");
					if (!filepath.empty())
					{
						SceneSerializer::Serialize(m_pActiveScene, filepath);
					}
					//ForceReset Keyboard!
				}
				else if (key == RLS_KEY::I)
				{
					if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
						m_ImmersiveModeEnabled = !m_ImmersiveModeEnabled;
				}
				else if (key == RLS_KEY::S && Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
				{
					SaveScene(std::string(EDITOR_ASSET_DIRECTORY) + "Scenes/Example.Relentless");
				}
			}
			event.StopPropagation();
			break;
		}
		}
	}

	void EditorLayer::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		UI_DrawMainMenuBar();

		ImGuiWindowClass viewportWindowClass;
		viewportWindowClass.DockNodeFlagsOverrideSet = m_ImmersiveModeEnabled ? ImGuiDockNodeFlags_NoTabBar : 0;
		ImGui::SetNextWindowClass(&viewportWindowClass);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Scene");

		UI_DrawSceneStateIcons();

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

		if (m_ViewportPanelSize.x != ImGui::GetContentRegionAvail().x || m_ViewportPanelSize.y != ImGui::GetContentRegionAvail().y)
		{
			//Verify values are valid (as is not the case when shutting down the program!)
			if (!(ImGui::GetContentRegionAvail().x < 0.0f) && !(ImGui::GetContentRegionAvail().y < 0.0f))
			{
				m_ViewportPanelSize.x = ImGui::GetContentRegionAvail().x;
				m_ViewportPanelSize.y = ImGui::GetContentRegionAvail().y;
				m_SceneViewportChanged = true;
			}
		}

		auto UITextureHandle = MasterRenderer::GetFrameBuffer()->GetOutput(0)->GetSRVDescriptorHandle().GPUHandle;
		ImGui::Image
		(
			(ImTextureID)UITextureHandle.ptr,
			ImVec2(m_ViewportPanelSize.x, m_ViewportPanelSize.y)
		);

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM_SCENE"))
			{
				const char* path = (const char*)payLoad->Data;
				m_Path = std::string(path);
			}
			ImGui::EndDragDropTarget();
		}

		if (m_SelectedEntity != NULL_ENTITY && m_CurrentGizmoType != GizmoType::NONE)
		{
			ManipulateTransformGizmo();
		}

		ImGui::End();
		ImGui::PopStyleVar();

		UI_DrawStatisticsPanel();

		m_SceneHierarchyPanel.OnImGuiRender(m_DisplaySceneHierarchyPanel && !m_ImmersiveModeEnabled);
		m_InspectorPanel.OnImGuiRender(m_DisplayInspectorPanel && !m_ImmersiveModeEnabled);
		m_SceneRendererPanel.OnImGuiRender(m_DisplaySceneRendererPanel && !m_ImmersiveModeEnabled);
		m_PropertiesPanel.OnImGuiRender(m_DisplayPropertiesPanel && !m_ImmersiveModeEnabled);
		m_ContentBrowserPanel.OnImGuiRender(m_DisplayContentBrowserPanel && !m_ImmersiveModeEnabled);
		m_MetricsPanel.OnImGuiRender(m_DisplayMetricsPanel && !m_ImmersiveModeEnabled);
	}

	void EditorLayer::OnAttach() noexcept
	{
		m_pActiveScene = std::make_shared<Scene>();
		m_pActiveScene->SetViewportPanelSize(m_ViewportPanelSize);
		LoadStarterMeshes();
		CreateStartScene();
		m_pSceneRenderer = std::make_shared<SceneRenderer>(m_pActiveScene);

		m_PropertiesPanel.SetActiveScene(m_pActiveScene.get());
		m_SceneHierarchyPanel.SetActiveScene(m_pActiveScene.get());
		m_SceneRendererPanel.SetActiveRenderer(m_pSceneRenderer);

		m_pEditorScene = m_pActiveScene;

		//TODO: Collapse into one switch-case-callback-function
		m_SceneHierarchyPanel.SetOnEntityDestroyFunction([this](entity entityID)
			{
				if (entityID == m_SelectedEntity)
				{
					m_PropertiesPanel.SetSelectedEntity(NULL_ENTITY);
					m_SelectedEntity = NULL_ENTITY;
				}
				if (entityID == m_HoveredEntity)
					m_HoveredEntity = NULL_ENTITY;
			});
		m_SceneHierarchyPanel.SetOnEntityCreatedFunction([this](entity entityID)
			{
				if (m_SelectedEntity != NULL_ENTITY)
				{
					m_pActiveScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);
				}
				m_pActiveScene->GetEntityManager().Add<SelectedInEditorComponent>(entityID);
				m_PropertiesPanel.SetSelectedEntity(entityID);
				m_SelectedEntity = entityID;

				if (m_CurrentGizmoType == GizmoType::NONE)
				{
					m_CurrentGizmoType = GizmoType::TRANSLATE;
				}
			});
		m_SceneHierarchyPanel.SetOnEntitySelectedFunction([this](entity entityID)
			{
				if (m_SelectedEntity != NULL_ENTITY)
				{
					m_pActiveScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);
				}
				m_pActiveScene->GetEntityManager().Add<SelectedInEditorComponent>(entityID);
				m_PropertiesPanel.SetSelectedEntity(entityID);
				m_SelectedEntity = entityID;

				if (m_CurrentGizmoType == GizmoType::NONE)
				{
					m_CurrentGizmoType = GizmoType::TRANSLATE;
				}
			});

		m_PropertiesPanel.SetOnMaterialSelectedCallback([this](const AssetHandle& materialHandle)
			{
				m_InspectorPanel.SetContext(materialHandle, InspectedAssetType::MATERIAL);
				m_DisplayInspectorPanel = true;
			});

		m_ContentBrowserPanel.SetOnAssetSelectedCallback([this](const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)
			{
				m_InspectorPanel.SetContext(AssetHandle, inspectedAssetType);
				if (inspectedAssetType == InspectedAssetType::NONE)
				{
					m_DisplayInspectorPanel = false;
				}
				else
				{
					m_DisplayInspectorPanel = true;
				}
			});

		

		m_PlayButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\PlayButton.png"), "");
		m_StopButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\StopButton.png"), "");
		m_PauseButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\PauseButton.png"), "");
		m_SimulateButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\SimulateButton.png"), "");
		m_StepButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\StepButton.png"), "");
	}

	void EditorLayer::OnUpdate(const float deltaTime) noexcept
	{
		PROFILE_FUNC;

		if (m_SceneViewportChanged)
			OnSceneViewportChanged();

		//CameraController should handle all this (and more for an even better editor camera experience!)
		const bool isNavigatingViewport = m_HoveringSceneViewport && Mouse::IsButtonPressed(RLS_BUTTON::Right);
		if (isNavigatingViewport && 
			(		Keyboard::IsKeyPressed(RLS_KEY::W) 
				||	Keyboard::IsKeyPressed(RLS_KEY::S) 
				||	Keyboard::IsKeyPressed(RLS_KEY::A) 
				||	Keyboard::IsKeyPressed(RLS_KEY::D)
				||  Keyboard::IsKeyPressed(RLS_KEY::Q)
				||  Keyboard::IsKeyPressed(RLS_KEY::E)))
		{
			m_pActiveScene->GetEditorCamera()->Update(deltaTime);
		}

		auto& cb = *m_pActiveScene->GetEditorCamera()->m_pConstantBuffer;
		MemoryManager::Get().UpdateConstantBuffer(cb, &m_pActiveScene->GetEditorCamera()->GetPosition());

		m_pActiveScene->OnUpdate(deltaTime);
	}

	void EditorLayer::OnRender() noexcept
	{
		PROFILE_FUNC;

		m_pSceneRenderer->Begin();
		m_pSceneRenderer->IssueRenderPasses();
		m_pSceneRenderer->End();
	}

	//This is post issuing the render commands.
	//Important: That does NOT mean it is finished.
	//To be specific: It should be assumed it is not
	void EditorLayer::OnPostRender() noexcept
	{
		PROFILE_FUNC;

		if (m_HoveringSceneViewport)
		{
			m_HoveredEntity = m_pSceneRenderer->GetHoveredEntity();
		}
		else
			m_HoveredEntity = NULL_ENTITY;

		m_SceneRendererPanel.OnPostRender();

		//TODO: CHANGE!!!!! FFS
		if (!m_Path.empty())
		{
			LoadScene(m_Path);
			m_Path = {};
		}
		else if (m_CreateNewScene)
		{
			MasterRenderer::WaitAndSyncAllFramesInFlight();
			m_pActiveScene = std::make_shared<Scene>();
			m_pActiveScene->SetViewportPanelSize(m_ViewportPanelSize);
			m_pSceneRenderer->SetContext(m_pActiveScene);

			m_PropertiesPanel.SetSelectedEntity(NULL_ENTITY);
			m_PropertiesPanel.SetActiveScene(m_pActiveScene.get());

			m_SceneHierarchyPanel.SetSelectedEntity(NULL_ENTITY);
			m_SceneHierarchyPanel.SetActiveScene(m_pActiveScene.get());

			m_SelectedEntity = m_HoveredEntity = NULL_ENTITY;

			m_SceneViewportChanged = true;
			m_CreateNewScene = false;
		}
	}

	void EditorLayer::LoadStarterMeshes() noexcept
	{
		const std::array<std::string, 6> starterMeshes
		{
			"Cube.rasset",
			"Capsule.rasset",
			"Cone.rasset",
			"Cylinder.rasset",
			"Icosphere.rasset",
			"Plane.rasset",
			//"Quad.rasset",
			//"Sphere.rasset",
			//"Torus.rasset",
			//"Triangle.rasset"//,
			//"UtahTeapot.rasset"
		};
		
		const std::string meshPath = std::string(ENGINE_ASSET_DIRECTORY) + std::string("Models\\StarterContent\\");

		//std::for_each(std::execution::par, starterMeshes.begin(), starterMeshes.end(), [&](std::string& starterMeshName)
		//	{
		//		std::string fullMeshPath(meshPath + std::string(starterMeshName));
		//		Serializer::Deserialize<Mesh>(fullMeshPath);
		//	});

		MeshImportSettings importSettings = {};
		importSettings.ImportMaterialsAndTextures = false;
		for (auto& mesh : starterMeshes)
		{
			const std::string fullMeshPath(meshPath + std::string(mesh));
			const bool succeeded = Serializer::Deserialize<Mesh>(fullMeshPath);
			RLS_VERIFY(succeeded, "Failed to deserialize Mesh engine asset '{0}'", mesh.c_str());
		}
	}

	void EditorLayer::CreateStartScene() noexcept
	{
		SceneSerializer::Deserialize(m_pActiveScene, ENGINE_ASSET_DIRECTORY + std::string("Scenes\\StarterScene.Relentless"));
	}

	void EditorLayer::OnSceneViewportChanged() noexcept
	{
		m_ViewportPanelSize.x = std::max(1.0f, m_ViewportPanelSize.x);
		m_ViewportPanelSize.y = std::max(1.0f, m_ViewportPanelSize.y);
		m_pActiveScene->SetViewportPanelSize(m_ViewportPanelSize);

		m_pActiveScene->GetEditorCamera()->RecalculateProjectionMatrix(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_pSceneRenderer->OnSceneViewportChanged(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
		m_SceneViewportChanged = false;
	}

	void EditorLayer::DestroySelectedEntity() noexcept
	{
		if (m_SelectedEntity == NULL_ENTITY)
			return;

		m_pActiveScene->DestroyEntity(m_SelectedEntity);
		m_SceneHierarchyPanel.SetSelectedEntity(NULL_ENTITY);
		m_PropertiesPanel.SetSelectedEntity(NULL_ENTITY);
		m_SelectedEntity = NULL_ENTITY;
	}

	//Should be extended to perform full copies, however as for now
	//it serves as an easy way of getting more meshes/models on screen.
	void EditorLayer::CopySelectedEntity() noexcept
	{
		if (m_SelectedEntity == NULL_ENTITY)
			return;

		auto& mgr = m_pActiveScene->GetEntityManager();
		auto newEntity = m_pActiveScene->CreateEntityWithUUID(mgr.Get<NameComponent>(m_SelectedEntity).Name.c_str(), IDComponent().UuId);
		auto& tc1 = mgr.Get<TransformComponent>(m_SelectedEntity);
		auto& tc2 = mgr.Get<TransformComponent>(newEntity);
		tc2.Translation = tc1.Translation;
		tc2.Rotation = tc1.Rotation;
		tc2.Scale = tc1.Scale;
		tc2.Transform = tc1.Transform;

		if (mgr.Has<MeshFilterComponent>(m_SelectedEntity))
		{
			auto& mfcNew = mgr.Add<MeshFilterComponent>(newEntity);
			auto& mfc = mgr.Get<MeshFilterComponent>(m_SelectedEntity);

			mfcNew.AssetHandle = mfc.AssetHandle;
		}
		if (mgr.Has<MeshRendererComponent>(m_SelectedEntity))
		{
			auto& mrcNew = mgr.Add<MeshRendererComponent>(newEntity);
			auto& mrc = mgr.Get<MeshRendererComponent>(m_SelectedEntity);
			mrcNew.AssetHandle = mrc.AssetHandle;
			mgr.Add<DirtyMeshRendererComponent>(newEntity);
		}
		if (mgr.Has<OpaquePassComponent>(m_SelectedEntity))
			mgr.Add<OpaquePassComponent>(newEntity);
		if (mgr.Has<DirectionalLightComponent>(m_SelectedEntity))
		{
			auto& newDlc = mgr.Add<DirectionalLightComponent>(newEntity);
			auto& dlc = mgr.Get<DirectionalLightComponent>(m_SelectedEntity);
			m_pActiveScene->GetLightManager().AllocateDirectionalLight(newEntity);
			newDlc.Color = dlc.Color;
			newDlc.Intensity = dlc.Intensity;
		}
		else if (mgr.Has<PointLightComponent>(m_SelectedEntity))
		{
			auto& newPlc = mgr.Add<PointLightComponent>(newEntity);
			auto& plc = mgr.Get<PointLightComponent>(m_SelectedEntity);
			m_pActiveScene->GetLightManager().AllocatePointLight(newEntity);
			newPlc.Color = plc.Color;
			newPlc.Intensity = plc.Intensity;
		}

		mgr.Remove<SelectedInEditorComponent>(m_SelectedEntity);
		mgr.AddOrReplace<SelectedInEditorComponent>(newEntity);

		m_SelectedEntity = newEntity;
		m_SceneHierarchyPanel.SetSelectedEntity(m_SelectedEntity);
		m_PropertiesPanel.SetSelectedEntity(m_SelectedEntity);
	}

	void EditorLayer::ManipulateTransformGizmo() noexcept
	{
		ImGuizmo::SetOrthographic(false);
		ImGuizmo::SetDrawlist();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());

		ImGuizmo::MODE mode;
		if (m_CurrentGizmoType == GizmoType::SCALE)
		{
			mode = ImGuizmo::LOCAL;
		}
		else
		{
			mode = (ImGuizmo::MODE)m_CurrentGizmoMode;
		}

		auto& tc = m_pActiveScene->GetEntityManager().Get<TransformComponent>(m_SelectedEntity);

		DirectX::XMFLOAT4X4 visualizationMatrix;
		visualizationMatrix = tc.Transform;
		ImGuizmo::Manipulate(*m_pActiveScene->GetEditorCamera()->GetViewMatrix().m, *m_pActiveScene->GetEditorCamera()->GetProjectionMatrix().m, (ImGuizmo::OPERATION)m_CurrentGizmoType, mode, *visualizationMatrix.m);
		if (ImGuizmo::IsUsing())
		{
			if (m_pActiveScene->GetEntityManager().Has<IsChildComponent>(m_SelectedEntity) && mode == ImGuizmo::LOCAL)
			{
				entity parent = m_pActiveScene->GetEntityManager().Get<IsChildComponent>(m_SelectedEntity).Parent;
				auto& parentTC = m_pActiveScene->GetEntityManager().Get<TransformComponent>(parent);
				DirectX::XMMATRIX parentTransform = DirectX::XMLoadFloat4x4(&parentTC.Transform);
				DirectX::XMMATRIX inverseParentTransform = DirectX::XMMatrixInverse(nullptr, parentTransform);
				DirectX::XMMATRIX visMatrix = DirectX::XMLoadFloat4x4(&visualizationMatrix);
				DirectX::XMMATRIX result = DirectX::XMMatrixMultiply(visMatrix, inverseParentTransform);
				DirectX::XMStoreFloat4x4(&visualizationMatrix, result);
			}

			DirectX::XMFLOAT3 translation, rotation, scale;
			ImGuizmo::DecomposeMatrixToComponents(&visualizationMatrix.m[0][0], &translation.x, &rotation.x, &scale.x);
			if (mode == ImGuizmo::WORLD)
			{
				tc.Translation = translation;
				float deltaRotationX = rotation.x - tc.Rotation.x;
				float deltaRotationY = rotation.y - tc.Rotation.y;
				float deltaRotationZ = rotation.z - tc.Rotation.z;
				tc.Rotation.x += deltaRotationX;
				tc.Rotation.y += deltaRotationY;
				tc.Rotation.z += deltaRotationZ;
				tc.Scale = scale;

				m_pActiveScene->GetEntityManager().AddOrReplace<DirtyTransformComponent>(m_SelectedEntity).AdjustedWorldSpace = true;
			}
			else
			{
				tc.LocalTranslation = translation;
				float deltaRotationX = rotation.x - tc.LocalRotation.x;
				float deltaRotationY = rotation.y - tc.LocalRotation.y;
				float deltaRotationZ = rotation.z - tc.LocalRotation.z;
				tc.LocalRotation.x += deltaRotationX;
				tc.LocalRotation.y += deltaRotationY;
				tc.LocalRotation.z += deltaRotationZ;
				tc.LocalScale = scale;

				m_pActiveScene->GetEntityManager().AddOrReplace<DirtyTransformComponent>(m_SelectedEntity).AdjustedWorldSpace = false;
			}
		}
	}

	void EditorLayer::LoadScene(const std::filesystem::path& filepath) noexcept
	{
		MasterRenderer::WaitAndSyncAllFramesInFlight();
		m_pActiveScene = std::make_shared<Scene>();
		m_pActiveScene->SetViewportPanelSize(m_ViewportPanelSize);
		SceneSerializer::Deserialize(m_pActiveScene, filepath.string());
		m_pSceneRenderer->SetContext(m_pActiveScene);

		m_PropertiesPanel.SetSelectedEntity(NULL_ENTITY);
		m_PropertiesPanel.SetActiveScene(m_pActiveScene.get());

		m_SceneHierarchyPanel.SetSelectedEntity(NULL_ENTITY);
		m_SceneHierarchyPanel.SetActiveScene(m_pActiveScene.get());

		m_SelectedEntity = m_HoveredEntity = NULL_ENTITY;

		m_SceneViewportChanged = true;
	}

	void EditorLayer::SaveScene(const std::filesystem::path& filepath) noexcept
	{
		SceneSerializer::Serialize(m_pActiveScene, filepath.string());
	}

	void EditorLayer::OnScenePlay() noexcept
	{
		Application::Get().SubmitToMainThread([&]()
			{
				MasterRenderer::WaitAndSyncAllFramesInFlight();

				m_SceneState = SceneState::Play;

				if (m_SelectedEntity != NULL_ENTITY)
				{
					m_pActiveScene->GetEntityManager().Remove<SelectedInEditorComponent>(m_SelectedEntity);
					m_SelectedEntity = NULL_ENTITY;
				}

				m_pActiveScene = Scene::Copy(m_pEditorScene);
				m_pActiveScene->OnRuntimeStart();
				
				SetSceneContext(m_pActiveScene);
			});
	}

	void EditorLayer::OnSceneStop() noexcept
	{
		Application::Get().SubmitToMainThread([&]()
			{
				MasterRenderer::WaitAndSyncAllFramesInFlight();

				m_SceneState = SceneState::Edit;
				m_pActiveScene->OnRuntimeStop();
				m_pActiveScene = m_pEditorScene;

				SetSceneContext(m_pActiveScene);

				m_SelectedEntity = NULL_ENTITY;
			});
	}

	void EditorLayer::SetSceneContext(std::shared_ptr<Scene> pScene) noexcept
	{
		RLS_ASSERT(pScene, "Scene is invalid.");

		m_pSceneRenderer->SetContext(pScene);

		m_PropertiesPanel.SetActiveScene(pScene.get());
		m_SceneHierarchyPanel.SetActiveScene(pScene.get());

		pScene->GetEditorCamera()->RecalculateProjectionMatrix(static_cast<uint32_t>(m_ViewportPanelSize.x), static_cast<uint32_t>(m_ViewportPanelSize.y));
	}

	void EditorLayer::UI_DrawStatisticsPanel() noexcept
	{
		if (!m_DisplayStatisticsPanel || m_ImmersiveModeEnabled)
			return;

		ImGui::Begin("Stats");
		ImGui::Text("Hovered entity:");
		ImGui::SameLine();

		if (m_HoveredEntity != NULL_ENTITY && m_pActiveScene->GetEntityManager().Exists(m_HoveredEntity))
		{
			ImGui::Text("%s (%d)", m_pActiveScene->GetEntityManager().Get<NameComponent>(m_HoveredEntity).Name.c_str(), (m_HoveredEntity >> 12));
		}
		else
		{
			ImGui::Text("None");
		}
		ImGui::Text("Selected entity:");
		ImGui::SameLine();
		m_SelectedEntity == NULL_ENTITY ? ImGui::Text("None") : ImGui::Text("%s (%d)", m_pActiveScene->GetEntityManager().Get<NameComponent>(m_SelectedEntity).Name.c_str(), (m_SelectedEntity >> 12));

		ImGui::Text("#Shader bindable descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", MemoryManager::Get().GetShaderBindableDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#CBV/SRV/UAV descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", MemoryManager::Get().GetCBVSRVUAVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#RTV descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", MemoryManager::Get().GetRTVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#DSV descriptors: ");
		ImGui::SameLine();
		ImGui::Text("%d", MemoryManager::Get().GetDSVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#Constant buffer sets: ");
		ImGui::SameLine();
		ImGui::Text("%d", MemoryManager::Get().GetNrOfConstantBuffersInUse());

		ImGui::End();
	}

	void EditorLayer::UI_DrawMainMenuBar() noexcept
	{
		if (m_ImmersiveModeEnabled)
			return;

		if (!ImGui::BeginMainMenuBar())
			return;

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("New", "Ctrl+N"))
			{
				m_CreateNewScene = true;
			}

			if (ImGui::MenuItem("Open...", "Ctrl+O"))
			{
				//TODO: FIX!!
				RLS_ASSERT(false, "FIX!");

				//std::string filepath = FileDialogs::OpenFile("Relentless Scene (*.Relentless)\0*.Relentless\0");
				//if (!filepath.empty())
				//{
				//	m_Path = filepath;
				//}
			}

			if (ImGui::MenuItem("Save as...", "Ctrl+Shift+S"))
			{
				std::string filepath = FileDialogs::SaveFile("Relentless Scene (*.Relentless)\0*.Relentless\0");
				if (!filepath.empty())
				{
					SceneSerializer::Serialize(m_pActiveScene, filepath);
				}
			}

			//TODO: Make it so the application can be exited here!
			if (ImGui::MenuItem("Exit"))
			{
				//TODO
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View"))
		{
			ImGui::MenuItem("Scene Hierarchy Panel", nullptr, &m_DisplaySceneHierarchyPanel);
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

	void EditorLayer::UI_DrawSceneStateIcons() noexcept
	{
		constexpr const ImVec2 sceneStateIconsWindowSize = ImVec2(190.0f, 50.0f);

		const float mainViewportPanelWidth = ImGui::GetWindowContentRegionWidth();
		const float offsetX = (mainViewportPanelWidth / 2.0f) - (sceneStateIconsWindowSize.x / 2.0f);
		constexpr const float offsetY = 30.0f;

		ImVec2 windowPosition = ImGui::GetWindowPos();
		windowPosition = ImVec2(windowPosition.x + offsetX, windowPosition.y + offsetY);

		auto& colors = ImGui::GetStyle().Colors;
		const auto& windowBGColor = colors[ImGuiCol_WindowBg];
		const auto& buttonColor = colors[ImGuiCol_Button];
		const auto& buttonActiveColor = colors[ImGuiCol_ButtonActive];
		const auto& buttonHoveredColor = colors[ImGuiCol_ButtonHovered];
		
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(windowBGColor.x, windowBGColor.y, windowBGColor.z, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(buttonColor.x, buttonColor.y, buttonColor.z, 0.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(buttonActiveColor.x, buttonActiveColor.y, buttonActiveColor.z, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buttonHoveredColor.x, buttonHoveredColor.y, buttonHoveredColor.z, 0.5f));

		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowRounding, 10.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_ItemInnerSpacing, ImVec2(0.0f, 0.0f));

		ImGui::SetNextWindowSize(sceneStateIconsWindowSize);
		ImGui::Begin("##SceneStateIcons", nullptr, ImGuiWindowFlags_NoDecoration);
		ImGui::SetWindowPos(windowPosition);

		const ImVec2 playStopButtonPosition = ImVec2(ImGui::GetCursorPosX() + 10.0f, ImGui::GetCursorPosY());
		ImGui::SetCursorPosX(playStopButtonPosition.x);
		
		constexpr const ImVec2 buttonSize = ImVec2(50.0f, 50.0f);
		constexpr const float buttonGap = 10.0f;

		//Draw Play/Stop button
		const Texture2D& IconTexture = AssetManager::Get<Texture2D>(m_SceneState == SceneState::Edit ? m_PlayButtonTextureHandle : m_StopButtonTextureHandle);
		if (ImGui::ImageButton((ImTextureID)IconTexture.GetSRVDescriptorHandle().GPUHandle.ptr, buttonSize, ImVec2(0.0f, 0.0f), ImVec2(1, 1), 0))
		{
			if (m_SceneState == SceneState::Edit)
			{
				OnScenePlay();
			}
			else
			{
				OnSceneStop();
			}
		}

		//Simulate button:
		ImVec2 simulateButtonPosition = playStopButtonPosition;
		if (m_SceneState == SceneState::Edit)
		{
			simulateButtonPosition = ImVec2(simulateButtonPosition.x + buttonSize.x + buttonGap, simulateButtonPosition.y);
			ImGui::SetCursorPos(simulateButtonPosition);

			const Texture2D& simulateIconTexture = AssetManager::Get<Texture2D>(m_SimulateButtonTextureHandle);
			if (ImGui::ImageButton((ImTextureID)simulateIconTexture.GetSRVDescriptorHandle().GPUHandle.ptr, buttonSize, ImVec2(0.0f, 0.0f), ImVec2(1, 1), 0))
			{
				m_SceneState = SceneState::Simulate;
			}
		}

		//Pause button:
		ImVec2 pauseButtonPosition = simulateButtonPosition;
		if (m_SceneState == SceneState::Play || m_SceneState == SceneState::Simulate)
		{
			pauseButtonPosition = ImVec2(pauseButtonPosition.x + buttonSize.x + buttonGap, pauseButtonPosition.y);
			ImGui::SetCursorPos(pauseButtonPosition);

			const Texture2D& pauseIconTexture = AssetManager::Get<Texture2D>(m_PauseButtonTextureHandle);
			if (ImGui::ImageButton((ImTextureID)pauseIconTexture.GetSRVDescriptorHandle().GPUHandle.ptr, buttonSize, ImVec2(0.0f, 0.0f), ImVec2(1, 1), 0))
			{
				m_pActiveScene->SetPaused(!m_pActiveScene->IsPaused());
			}
		}

		//Step button:
		if (m_pActiveScene->IsPaused())
		{
			ImVec2 stepButtonPosition = pauseButtonPosition;
			stepButtonPosition = ImVec2(stepButtonPosition.x + buttonSize.x + buttonGap, stepButtonPosition.y);
			ImGui::SetCursorPos(stepButtonPosition);

			const Texture2D& stepIconTexture = AssetManager::Get<Texture2D>(m_StepButtonTextureHandle);
			if (ImGui::ImageButton((ImTextureID)stepIconTexture.GetSRVDescriptorHandle().GPUHandle.ptr, buttonSize, ImVec2(0.0f, 0.0f), ImVec2(1, 1), 0))
			{
				//TODO: Send step signal to scene....
			}
		}

		ImGui::PopStyleColor(4);
		ImGui::PopStyleVar(3);
		ImGui::End();
	}
}
