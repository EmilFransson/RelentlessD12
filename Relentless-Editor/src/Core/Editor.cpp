#include "Editor.h"
#include "RelentlessEditorApp.h"

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
				if (Mouse::IsButtonDown(RLS_Button::Left) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
				{
					m_IsPanningMouse = true;

					Mouse::ConfineCursor(vMin.x, vMax.x, vMax.y, vMin.y);
					Mouse::HideCursor();
					event.StopPropagation();
				}
			}

			break;
		}
		case EventType::LeftMouseButtonPressedEvent:
		case EventType::RightMouseButtonPressedEvent:
		case EventType::MiddleMouseButtonPressedEvent:
		{
			if (ViewportPanel* pViewport = GetHoveredViewport())
			{
				const Vector2u cursorScreenPosition = Mouse::GetCursorScreenPosition();
				Mouse::ConfineCursor(cursorScreenPosition.x, cursorScreenPosition.x, cursorScreenPosition.y, cursorScreenPosition.y);
				Mouse::HideCursor();
				event.StopPropagation();
			}
			break;
		}
		case EventType::RightMouseButtonReleasedEvent:
		case EventType::MiddleMouseButtonReleasedEvent:
		{
			if (ViewportPanel* pViewport = GetHoveredViewport())
			{
				Mouse::FreeCursor();
				Mouse::ShowCursor();
				event.StopPropagation();
			}

			break;
		}
		case EventType::LeftMouseButtonReleasedEvent:
		{
			ViewportPanel* pViewport = GetHoveredViewport();
			if (!pViewport)
				return;

			Mouse::FreeCursor();
			Mouse::ShowCursor();
			event.StopPropagation();
			m_IsPanningMouse = false;

			const bool lCtrlDown = Keyboard::IsKeyDown(RLS_Key::LCtrl);
			const bool lShiftDown = Keyboard::IsKeyDown(RLS_Key::LShift);
			const bool isHoveringEntity = m_HoveredEntity != NULL_ENTITY;

			if (!isHoveringEntity || (!lCtrlDown && !lShiftDown))
			{
				m_pOutlinerPanel->DeselectNonEntityItems();
				m_pSelection->DeselectAllEntities();
			}
			
			if (isHoveringEntity)
			{
				if (lCtrlDown && m_pSelection->IsEntitySelected(m_HoveredEntity))
					m_pSelection->DeselectEntity(m_HoveredEntity);
				else
					m_pSelection->SelectEntity(m_HoveredEntity);
			}

			break;
		}
		case EventType::KeyPressedEvent:
		{
			const bool isNavigatingScene = m_HoveringSceneViewport && Mouse::IsButtonDown(RLS_Button::Right);
			if (!isNavigatingScene)
			{
				const RLS_Key key = EVENT(KeyPressedEvent).key;
				switch (key)
				{
				case RLS_Key::Q: m_CurrentGizmoType = EGizmoType::None; break;
				case RLS_Key::W: m_CurrentGizmoType = (EGizmoType)ImGuizmo::TRANSLATE; break;
				case RLS_Key::E: m_CurrentGizmoType = (EGizmoType)ImGuizmo::ROTATE; break;
				case RLS_Key::R: m_CurrentGizmoType = (EGizmoType)ImGuizmo::SCALE; break;
				case RLS_Key::T: m_CurrentGizmoMode = (EGizmoMode)!(bool)m_CurrentGizmoMode; break;
				case RLS_Key::I:
				{
					if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
						m_ImmersiveModeEnabled = !m_ImmersiveModeEnabled;

					break;
				}
				case RLS_Key::A:
				{
					break;
					if (m_ViewportIsFocused)
					{
						if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
						{
							m_pActiveScene->GetEntityManager().Collect<IDComponent>().Do([this](entity e)
								{
									if (!m_pSelection->IsEntitySelected(e))
										m_pSelection->SelectEntity(e);
								});
						}
					}
					else if (m_pOutlinerPanel->IsFocused())
						m_pOutlinerPanel->SelectAllExpanded();

					break;
				}
				case RLS_Key::H:
				{
					if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
					{
						m_pActiveScene->GetEntityManager().Collect<HiddenInGameComponent>().Do([this](entity e)
							{
								m_pActiveScene->SetEntityVisibleInGame(e, true);
							});
					}
					else
					{
						const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();
						EntityManager& entityManager = m_pActiveScene->GetEntityManager();

						for (int i = selectedEntities.size() - 1; i >= 0; --i)
						{
							const entity currentEntity = selectedEntities[i];

							m_pSelection->DeselectEntity(currentEntity);
								m_pActiveScene->SetEntityVisibleInGame(currentEntity, false);
						}
					}
					break;
				}
				case RLS_Key::Delete:
				{
					if (m_ViewportIsFocused)
					{
						const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();
						EntityManager& entityManager = m_pActiveScene->GetEntityManager();

						for (int i = selectedEntities.size() - 1; i >= 0; --i)
						{
							const entity currentEntity = selectedEntities[i];
							m_pActiveScene->DestroyEntity(currentEntity);
						}
					}
					else if (m_pOutlinerPanel->IsFocused())
						m_pOutlinerPanel->OnDeleteKeyPressed();
				
					break;
				}
				}
			}
			event.StopPropagation();
			break;
		}
		}

		//m_ContentBrowserPanel.OnEvent(event);
	}

	void Editor::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		UI_DrawMainMenuBar();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main", nullptr);

		ImGui::BeginChild("Scene", ImVec2(0,60), false);

		ImGui::EndChild();

		ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		for (auto& viewport : m_EditorViewports)
			viewport->Render();

		ImGui::Begin("Content Browser");

		ImGui::End();

		ImGui::Begin("Outliner");

		ImGui::End();

		//UI_DrawMainMenuBar();
		//
		//ImGuiWindowClass viewportWindowClass;
		//viewportWindowClass.DockNodeFlagsOverrideSet = m_ImmersiveModeEnabled ? ImGuiDockNodeFlags_NoTabBar : 0;
		//ImGui::SetNextWindowClass(&viewportWindowClass);
		//
		//ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		//ImGui::Begin("Scene");
		//m_ViewportIsFocused = ImGui::IsWindowFocused();
		//
		////UI_DrawSceneStateIcons();
		//
		//if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(0))
		//{
		//	m_DisplayInspectorPanel = false;
		//}
		//
		//vMin = ImGui::GetWindowContentRegionMin();
		//vMax = ImGui::GetWindowContentRegionMax();
		//vMin.x += ImGui::GetWindowPos().x;
		//vMin.y += ImGui::GetWindowPos().y;
		//vMax.x += ImGui::GetWindowPos().x;
		//vMax.y += ImGui::GetWindowPos().y;
		//
		//m_HoveringSceneViewport = ImGui::IsWindowHovered();
		//
		//const float toolBarPadding = m_ImmersiveModeEnabled ? 0 : 24.0f;
		//ImVec2 mousePosition = ImGui::GetMousePos();
		//ImVec2 windowPosition = ImGui::GetWindowPos();
		//ImVec2 mousePositionInSceneClientArea = ImVec2(mousePosition.x - windowPosition.x, mousePosition.y - windowPosition.y - toolBarPadding);
		//m_pActiveScene->SetMousePosition(mousePositionInSceneClientArea);
		//auto xx = ImGui::GetContentRegionAvail().x;
		//auto yy = ImGui::GetContentRegionAvail().y;
		//
		//if (m_ViewportPanelSize.x != xx || m_ViewportPanelSize.y != yy)
		//{
		//	//Verify values are valid (as is not the case when shutting down the program!)
		//	if (!(ImGui::GetContentRegionAvail().x < 0.0f) && !(ImGui::GetContentRegionAvail().y < 0.0f))
		//	{
		//		m_ViewportPanelSize.x = ImGui::GetContentRegionAvail().x;
		//		m_ViewportPanelSize.y = ImGui::GetContentRegionAvail().y;
		//		m_SceneViewportChanged = true;
		//	}
		//}
		//
		//m_pSceneRenderer->OnImGuiRender(ImVec2(m_ViewportPanelSize.x, m_ViewportPanelSize.y));
		//
		//if (ImGui::BeginDragDropTarget())
		//{
		//	if (const ImGuiPayload* payLoad = ImGui::AcceptDragDropPayload("MULTIPLE_ENTRIES_DRAG_DROP"))
		//	{
		//		const std::vector<std::string>& selectedEntries = m_ContentBrowserPanel.GetSelectedEntries();
		//		std::for_each(selectedEntries.begin(), selectedEntries.end(), [this](const std::string& path)
		//			{
		//				if (!AssetRegistry::IsFilepathMapped(path))
		//					return;
		//
		//		const AssetHandle handle = AssetManager::GetHandleByPath(path);
		//		if (handle.Type != AssetType::Mesh)
		//			return;
		//
		//		CreateEntityFromDroppedMesh(handle);
		//			});
		//	}
		//	ImGui::EndDragDropTarget();
		//}
		//
		//if (m_CurrentGizmoType != EGizmoType::None)
		//	ManipulateTransformGizmo();
		//
		//ImGui::End();
		//ImGui::PopStyleVar();
		//
		//UI_DrawStatisticsPanel();
		//
		//m_pOutlinerPanel->OnImGuiRender(m_DisplayOutlinerPanel && !m_ImmersiveModeEnabled);
		//m_InspectorPanel.OnImGuiRender(m_DisplayInspectorPanel && !m_ImmersiveModeEnabled);
		//m_SceneRendererPanel.OnImGuiRender(m_DisplaySceneRendererPanel && !m_ImmersiveModeEnabled);
		//m_PropertiesPanel.OnImGuiRender(m_DisplayPropertiesPanel && !m_ImmersiveModeEnabled);
		//m_ContentBrowserPanel.OnImGuiRender(m_DisplayContentBrowserPanel && !m_ImmersiveModeEnabled);
		//m_MetricsPanel.OnImGuiRender(m_DisplayMetricsPanel && !m_ImmersiveModeEnabled);
	}

	void Editor::OnCreate() noexcept
	{
		m_pSelection = std::make_unique<Selection>();
		m_pEntityFiltersManager = std::make_unique<EntityFiltersManager>();

		m_RenderViews.push_back(ViewportRenderView());
		m_EditorViewports.push_back(std::make_unique<ViewportPanel>(std::format("Scene Viewport {}", m_EditorViewports.size()+1).c_str(), ImGuiWindowFlags_None, this, m_EditorViewports.size()));
		
		{
			std::shared_ptr<PerspectiveCamera> pEditorCamera = PerspectiveCamera::Create();
			pEditorCamera->SetLocation(Vector3(5.0f, 5.0f, -5.0f));
			pEditorCamera->SetNearPlane(0.1f);
			pEditorCamera->SetFarPlane(1'000.0f);
			pEditorCamera->SetRotation(Quaternion::CreateFromYawPitchRoll(-Math::PI_DIV_4, Math::PI_DIV_4 * 0.5f, 0));
			m_ViewportCameras.push_back(pEditorCamera);
		}

		m_RenderViews.push_back(ViewportRenderView());
		m_EditorViewports.push_back(std::make_unique<ViewportPanel>(std::format("Scene Viewport {}", m_EditorViewports.size() + 1).c_str(), ImGuiWindowFlags_None, this, m_EditorViewports.size()));

		{
			std::shared_ptr<PerspectiveCamera> pEditorCamera = PerspectiveCamera::Create();
			pEditorCamera->SetLocation(Vector3(0.0f, 5.0f, -5.0f));
			pEditorCamera->SetNearPlane(0.1f);
			pEditorCamera->SetFarPlane(1'000.0f);
			pEditorCamera->SetRotation(Quaternion::Identity);
			m_ViewportCameras.push_back(pEditorCamera);
		}

		m_pOutlinerPanel = std::make_unique<OutlinerPanel>(this);
		SetActiveScene(std::make_shared<Scene>());

		entity dirEntity = m_pActiveScene->CreateEntity("Directional Light");
		auto& dlc = m_pActiveScene->GetEntityManager().Add<DirectionalLightComponent>(dirEntity);
		dlc.Color = Math::MakeFromColorTemperature(5900.0f);
		dlc.Intensity = 3.0f;
		Vector3 target = Vector3(0, 0, 0);
		Vector3 from = Vector3(7, 5, 5);
		dlc.Direction = target - from;
		dlc.Direction.Normalize();

		Ref<Material> pWhiteMaterial = new Material();
		pWhiteMaterial->SetRenderMode(RenderMode::Opaque);
		pWhiteMaterial->m_AlbedoColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		pWhiteMaterial->m_Metallic = 0.0f;
		pWhiteMaterial->m_Roughness = 0.5f;

		const uint32 index = AssetManager::GetStorage<Material>().Add(pWhiteMaterial);
		auto [handle, _] = AssetManager::InsertMetaData(CreateUUID(), index, AssetType::Material);
		
		Ref<Material> pNewWhiteMaterial = new Material();
		pNewWhiteMaterial->SetRenderMode(RenderMode::Opaque);
		pNewWhiteMaterial->m_AlbedoColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
		pNewWhiteMaterial->m_Metallic = 0.0f;
		pNewWhiteMaterial->m_Roughness = 1.0f;
		const uint32 index2 = AssetManager::GetStorage<Material>().Add(pNewWhiteMaterial);
		auto [handle2, __] = AssetManager::InsertMetaData(CreateUUID(), index2, AssetType::Material);

		entity ground = m_pActiveScene->CreateEntity("Ground");

		m_pActiveScene->AddWorldOffset(ground, Vector3(0.0f, -1.0f, 0.0f));
		m_pActiveScene->SetWorldScale(ground, Vector3(4.0f, 0.5f, 4.0f));

		{
			auto& mrc = m_pActiveScene->GetEntityManager().Add<MeshRendererComponent>(ground);
			mrc.AssetHandle = handle2->second;
		}

		entity e = m_pActiveScene->CreateEntity("Sphere");
		m_pActiveScene->GetEntityManager().Add<RotatorComponent>(e);

		auto& mrc = m_pActiveScene->GetEntityManager().Add<MeshRendererComponent>(e);
		mrc.AssetHandle = handle->second;

		{
			TextureImportSettings textureImportSettings;
			textureImportSettings.GenerateMipMaps = true;
			textureImportSettings.TextureCompressionType = ETextureCompressionType::Uncompressed;
			textureImportSettings.IsSRGB = true;
			textureImportSettings.IsHDR = false;

			std::vector<ImportRequest> requests;
			
			{
				MeshImportSettings meshImportSettings;
				meshImportSettings.OptimizeMesh = true;

				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = meshImportSettings;
				request.Filepath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Models/StarterContent/Sphere.obj");
				request.OnAssetImported.Connect([this, e](const AssetHandle& assetHandle, bool success)
					{
						auto& mfc = m_pActiveScene->GetEntityManager().Add<MeshFilterComponent>(e);
						mfc.AssetHandle = assetHandle;
					});
			}
			{
				MeshImportSettings meshImportSettings;
				meshImportSettings.OptimizeMesh = true;

				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = meshImportSettings;
				request.Filepath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Models/StarterContent/Cube.obj");
				request.OnAssetImported.Connect([this, ground](const AssetHandle& assetHandle, bool success)
					{
						auto& mfc = m_pActiveScene->GetEntityManager().Add<MeshFilterComponent>(ground);
						mfc.AssetHandle = assetHandle;
					});
			}
			{
				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = textureImportSettings;
				request.Filepath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/rusty-metal-ue/rusty-metal_albedo.png");
				request.OnAssetImported.Connect([this, e](const AssetHandle& assetHandle, bool success)
					{
						auto& mrc = m_pActiveScene->GetEntityManager().Get<MeshRendererComponent>(e);
						AssetManager::Get<Material>(mrc.AssetHandle)->SetAlbedoTexture(assetHandle);
					});
			}
			{
				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = textureImportSettings;
				textureImportSettings.IsSRGB = false;

				request.Filepath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/rusty-metal-ue/rusty-metal_metallic.png");
				request.OnAssetImported.Connect([this, e](const AssetHandle& assetHandle, bool success)
					{
						RLS_ASSERT(success, "FAILED!");
						auto& mrc = m_pActiveScene->GetEntityManager().Get<MeshRendererComponent>(e);
						AssetManager::Get<Material>(mrc.AssetHandle)->SetMetallicTexture(assetHandle);
					});
			}
			{
				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = textureImportSettings;
				textureImportSettings.IsSRGB = false;

				request.Filepath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/rusty-metal-ue/rusty-metal_roughness.png");
				request.OnAssetImported.Connect([this, e](const AssetHandle& assetHandle, bool success)
					{
						RLS_ASSERT(success, "FAILED!");
						auto& mrc = m_pActiveScene->GetEntityManager().Get<MeshRendererComponent>(e);
						AssetManager::Get<Material>(mrc.AssetHandle)->SetRoughnessTexture(assetHandle);
					});
			}
			{
				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = textureImportSettings;
				textureImportSettings.IsSRGB = false;

				request.Filepath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/rusty-metal-ue/rusty-metal_ao.png");
				request.OnAssetImported.Connect([this, e](const AssetHandle& assetHandle, bool success)
					{
						RLS_ASSERT(success, "FAILED!");
						auto& mrc = m_pActiveScene->GetEntityManager().Get<MeshRendererComponent>(e);
						AssetManager::Get<Material>(mrc.AssetHandle)->SetAmbientOcclusionTexture(assetHandle);
					});
			}
			{
				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = textureImportSettings;
				textureImportSettings.IsSRGB = false;

				request.Filepath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/rusty-metal-ue/rusty-metal_normal-dx.png");
				request.OnAssetImported.Connect([this, e](const AssetHandle& assetHandle, bool success)
					{
						RLS_ASSERT(success, "FAILED!");
						auto& mrc = m_pActiveScene->GetEntityManager().Get<MeshRendererComponent>(e);
						AssetManager::Get<Material>(mrc.AssetHandle)->SetNormalMap(assetHandle);
					});
			}
			{
				ImportRequest& request = requests.emplace_back();
				request.ImportSettings = textureImportSettings;
				textureImportSettings.IsSRGB = false;

				request.Filepath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, "Textures/rusty-metal-ue/rusty-metal_height.png");
				request.OnAssetImported.Connect([this, e](const AssetHandle& assetHandle, bool success)
					{
						RLS_ASSERT(success, "FAILED!");
						auto& mrc = m_pActiveScene->GetEntityManager().Get<MeshRendererComponent>(e);
						AssetManager::Get<Material>(mrc.AssetHandle)->SetHeightMap(assetHandle);
					});
			}

			Ref<ImporterFeedbackContext> pFeedback = new ImporterFeedbackContext();
		
			std::future<void> fut = Importer::RequestAsyncLoad(Application::Get().GetGraphicsDevice(), requests, pFeedback);
			fut.wait();
		}

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());
		app.GetRenderer()->OnEntityIDReadbackDone.Connect(this, &Editor::OnEntityReadbackDone);

		m_pSelection->OnSelectionChanged.Connect(this, &Editor::OnEntitySelectionChanged);
		
		//
		//LoadStarterMeshes();
		//CreateStartScene();
		//
		//m_pSceneRenderer = std::make_shared<SceneRenderer>(m_pActiveScene);
		//m_pUtilityRenderer = std::make_shared<UtilityRenderer>();
		//
		//m_PropertiesPanel.SetOnMaterialSelectedCallback([this](const AssetHandle& materialHandle)
		//	{
		//		m_InspectorPanel.SetContext(materialHandle, InspectedAssetType::MATERIAL);
		//		m_DisplayInspectorPanel = true;
		//	});
		//
		//m_ContentBrowserPanel.SetOnAssetSelectedCallback([this](const AssetHandle& AssetHandle, const InspectedAssetType inspectedAssetType)
		//	{
		//		m_InspectorPanel.SetContext(AssetHandle, inspectedAssetType);
		//		if (inspectedAssetType == InspectedAssetType::NONE)
		//			m_DisplayInspectorPanel = false;
		//		else
		//			m_DisplayInspectorPanel = true;
		//	});
		//
		////m_PlayButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\PlayButton.png"), "");
		//m_StopButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\StopButton.png"), "");
		//m_PauseButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\PauseButton.png"), "");
		//m_SimulateButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\SimulateButton.png"), "");
		//m_StepButtonTextureHandle = AssetManager::LoadFromFile<Texture2D>(EDITOR_RESOURCE_DIRECTORY + std::string("Icons\\StepButton.png"), "");
		//
		//const std::filesystem::path srcPath = FilepathUtils::Combine(ENGINE_ASSET_DIRECTORY, "Textures/puresky.rasset");
		//
		//AssetHandle handle;
		//if (AssetManager::RequestLoadAsset(srcPath, handle))
		//{
		//	std::shared_ptr<Texture2D> pTexture = AssetManager::Get<Texture2D>(handle);
		//	m_pUtilityRenderer->ConvertEquirectangularToCubeMap(pTexture, [this](std::shared_ptr<TextureCube> pTextureCube)
		//		{
		//			m_pActiveScene->m_pSkyBox = pTextureCube;
		//	m_pUtilityRenderer->CreateIrradianceMap(pTextureCube, [this, pTextureCube](std::shared_ptr<TextureCube> pIrradianceMap)
		//		{
		//			m_pActiveScene->m_pIrradianceMap = pIrradianceMap;
		//		});
		//	m_pUtilityRenderer->CreateRadianceMap(pTextureCube, [this, pTextureCube](std::shared_ptr<TextureCube> pRadianceMap)
		//		{
		//			m_pActiveScene->m_pRadianceMap = pRadianceMap;
		//		});
		//		});
		//}
		//
		//m_PropertiesPanel.SetActiveScene(m_pActiveScene.get());
		//m_SceneRendererPanel.SetActiveRenderer(m_pSceneRenderer);
	}

	void Editor::OnDestroy() noexcept
	{
		m_pSelection->OnSelectionChanged.Detach(this);

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());
		if (auto& pRenderer = app.GetRenderer())
		{
			pRenderer->OnEntityIDReadbackDone.Detach(this);
		}
	}

	void Editor::OnUpdate(const float deltaTime) noexcept
	{
		m_pActiveScene->GetEntityManager().Collect<TransformComponent, RotatorComponent>().Do([&](entity e)
			{
				const float rotationSpeed = 10.0f;
				const Vector3 rotation = Vector3(0, deltaTime * rotationSpeed, 0);
				m_pActiveScene->AddWorldRotation(e, rotation);
			});

		for (int i = 0; i < m_EditorViewports.size(); ++i)
		{
			UniquePtr<ViewportPanel>& pViewportPanel = m_EditorViewports[i];

			const Vector2u& region = pViewportPanel->GetViewportSize();
			m_RenderViews[i].Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));
			m_ViewportCameras[i]->SetViewport(m_RenderViews[i].Viewport);

			const bool cameraMovementEnabled = pViewportPanel->IsClientAreaHovered();
			m_ViewportCameras[i]->Update(cameraMovementEnabled);

			const ViewTransform& cameraViewTransform = m_ViewportCameras[i]->GetViewTransform();
			ViewportRenderView& renderView = m_RenderViews[i];
			
			renderView.Location					= cameraViewTransform.Location;
			renderView.Viewport					= cameraViewTransform.Viewport;
			renderView.IsPerspective			= true;
			renderView.PerspectiveFrustum		= cameraViewTransform.PerspectiveFrustum;
			renderView.OrthographicFrustum		= cameraViewTransform.OrthographicFrustum;

			renderView.WorldToView				= cameraViewTransform.WorldToView;
			renderView.WorldToClip				= cameraViewTransform.WorldToClip;
			renderView.ViewToWorld				= cameraViewTransform.ViewToWorld;
			renderView.ViewToClip				= cameraViewTransform.ViewToClip;
			renderView.ClipToView				= cameraViewTransform.ClipToView;

			renderView.FoV						= cameraViewTransform.FoV;
			renderView.NearPlane				= cameraViewTransform.NearPlane;
			renderView.FarPlane					= cameraViewTransform.FarPlane;

			renderView.MouseHoverCoordinates = pViewportPanel->IsClientAreaHovered() ? pViewportPanel->GetClientHoverCoordinates() : Vector2i(-1, -1);
		}

		//PROFILE_FUNC;
		//
		//if (m_SceneViewportChanged)
		//	OnSceneViewportChanged();
		//
		//m_pActiveScene->GetEditorCamera()->Update();
		//m_pActiveScene->OnUpdate(deltaTime);
	}

	void Editor::OnRender() noexcept
	{
		//PROFILE_FUNC;
		//
		//m_pSceneRenderer->Begin();
		//m_pSceneRenderer->IssueRenderPasses();
		//m_pSceneRenderer->End();
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

		//m_SceneRendererPanel.OnPostRender();
	}

	Scene* Editor::GetActiveScene() const noexcept
	{
		return m_pActiveScene.get();
	}

	const UniquePtr<Selection>& Editor::GetSelection() noexcept
	{
		return m_pSelection;
	}

	const UniquePtr<EntityFiltersManager>& Editor::GetEntityFiltersManager() noexcept
	{
		return m_pEntityFiltersManager;
	}

	ViewportRenderView& Editor::GetRenderView(uint32 renderViewIndex) noexcept
	{
		RLS_ASSERT(m_RenderViews.size() > renderViewIndex, "[Editor::GetRenderView] Index Out Of Bounds Error.");
		return m_RenderViews[renderViewIndex];
	}

	std::vector<ViewportRenderView>& Editor::GetRenderViews() noexcept
	{
		return m_RenderViews;
	}

	bool Editor::IsHoveringAnyFocusedViewport() const noexcept
	{
		return std::any_of(m_EditorViewports.begin(), m_EditorViewports.end(), [](const UniquePtr<ViewportPanel>& pViewport)
			{
				return pViewport->IsFocused() && pViewport->IsHovered();
			});
	}

	bool Editor::IsNavigatingAnyViewport() const noexcept
	{
		return std::any_of(m_EditorViewports.begin(), m_EditorViewports.end(), [](const UniquePtr<ViewportPanel>& pViewport)
			{
				return pViewport->IsFocused() && pViewport->IsHovered() && 
					(Mouse::IsButtonDown(RLS_Button::Right) || Mouse::IsButtonDown(RLS_Button::Left) || Mouse::IsButtonDown(RLS_Button::Wheel));
			});
	}

	ViewportPanel* Editor::GetHoveredViewport() const noexcept
	{
		for (auto& pViewport : m_EditorViewports)
		{
			if (pViewport->IsClientAreaHovered())
				return pViewport.get();
		}

		return nullptr;
	}

	void Editor::SetActiveScene(const std::shared_ptr<Scene>& pScene) noexcept
	{
		m_pSelection->DeselectAllEntities();

		if (m_pActiveScene)
		{
			m_pActiveScene->GetEntityManager().Collect<IDComponent>().Do([this](entity e)
				{
					if (m_pEntityFiltersManager->IsEntityInAnyFilter(e))
						m_pEntityFiltersManager->RemoveEntityFromCurrentFilter(e);
				});

			m_pActiveScene->OnEntityPreDestroyed.Detach(this);
			m_pActiveScene->OnEntityAttached.Detach(this);
		}

		m_pActiveScene = pScene;
		m_pEditorScene = m_pActiveScene;

		m_pActiveScene->SetViewportPanelSize(m_ViewportPanelSize);

		m_pActiveScene->OnEntityPreDestroyed.Connect(this, &Editor::OnEntityPreDestroyed);
		m_pActiveScene->OnEntityAttached.Connect(this, &Editor::OnEntityAttached);

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
		const entity parent = m_pActiveScene->CreateShape(Shape::Cube);
		const entity child = m_pActiveScene->CreateShape(Shape::Torus);
		const entity otherCube = m_pActiveScene->CreateShape(Shape::Capsule);
		const entity other = m_pActiveScene->CreateShape(Shape::Sphere);
		m_pActiveScene->SetWorldLocation(parent, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pActiveScene->SetWorldLocation(child, DirectX::XMFLOAT3(5.0f, 0.0f, 0.0f));
		m_pActiveScene->SetWorldLocation(otherCube, DirectX::XMFLOAT3(-5.0f, 0.0f, 0.0f));
		m_pActiveScene->SetWorldLocation(other, DirectX::XMFLOAT3(-8.0f, 0.0f, 0.0f));

		m_pEntityFiltersManager->CreateFilter("StarterContent/Shapes/Cubes/Another/Last");
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
		const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();
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

		const bool manipulated = ImGuizmo::Manipulate(*m_pActiveScene->GetEditorCamera()->GetViewTransform().WorldToView.m, *m_pActiveScene->GetEditorCamera()->GetViewTransform().ViewToClip.m, (ImGuizmo::OPERATION)m_CurrentGizmoType, mode, pivot.m[0]);
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

		//m_PropertiesPanel.SetActiveScene(pScene.get());

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
		//ImGui::Text("%d", Application::Get().GetMemorymanager().GetShaderBindableDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#CBV/SRV/UAV descriptors: ");
		ImGui::SameLine();
		//ImGui::Text("%d", Application::Get().GetMemorymanager().GetCBVSRVUAVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#RTV descriptors: ");
		ImGui::SameLine();
		//ImGui::Text("%d", Application::Get().GetMemorymanager().GetRTVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#DSV descriptors: ");
		ImGui::SameLine();
		//ImGui::Text("%d", Application::Get().GetMemorymanager().GetDSVDescriptorHeap()->GetNrOfDescriptorsInUse());

		ImGui::Text("#Constant buffer sets: TODO!!");

		ImGui::End();
	}

	void Editor::UI_DrawMainMenuBar() noexcept
	{
		if (m_ImmersiveModeEnabled)
			return;

		if (!ImGui::BeginMainMenuBar())
			return;

		const ImVec2 menuBarPos = ImGui::GetWindowPos();
		const ImVec2 menuBarSize = ImGui::GetWindowSize();

		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit", nullptr))
				Application::Get().InitializeShutdownProcedure();

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

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();

		static bool isDragging = false;
		static Vector2u dragStartCursorPos = {};
		static Vector2u dragStartWindowPos = {};

		const ImVec2 vMax = ImVec2(menuBarPos.x + menuBarSize.x, menuBarPos.y + menuBarSize.y);
		const bool hovering = ImGui::IsMouseHoveringRect(menuBarPos, vMax, false);
		
		if (hovering && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			if (!isDragging)
			{
				isDragging = true;
				dragStartCursorPos = Mouse::GetCursorScreenPosition();
				dragStartWindowPos = Application::Get().GetWindow()->GetTopLeft();
			}
			else
			{
				const Vector2u currentCursorPos = Mouse::GetCursorScreenPosition();
				const Vector2u delta = currentCursorPos - dragStartCursorPos;
				Application::Get().GetWindow()->SetPosition(dragStartWindowPos + delta);
			}
		}
		else
		{
			isDragging = false;
		}
	}

	void Editor::CreateEntityFromDroppedMesh(const AssetHandle& meshHandle) noexcept
	{
		Ref<Mesh> pMesh = AssetManager::Get<Mesh>(meshHandle);
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
		if (m_pSelection->IsEntitySelected(e))
			m_pSelection->DeselectEntity(e);

		if (m_pEntityFiltersManager->IsEntityInAnyFilter(e))
			m_pEntityFiltersManager->RemoveEntityFromCurrentFilter(e);
	}

	void Editor::OnEntityAttached(entity child, entity parent) noexcept
	{
		if (m_pEntityFiltersManager->IsEntityInAnyFilter(child))
			m_pEntityFiltersManager->RemoveEntityFromCurrentFilter(child);
	}

	void Editor::OnEntityReadbackDone(uint32 entityID) noexcept
	{
		/*
		 An id of 0 is considered a sentinel value for the readback results.
		 This means all actual entity ids are shifted up by one and should be downshifted again (if entityID != 0)
		*/

		if (entityID == 0u)
			m_HoveredEntity = NULL_ENTITY;
		else
		{
			const uint32 actualEntityID = entityID - 1;
			m_HoveredEntity = m_pActiveScene->GetEntityManager().GetEntityFromIdentity(actualEntityID);
		}
	}
}
