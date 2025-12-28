#include "Editor.h"
#include "RelentlessEditorApp.h"

#include "../Panels/TestPanel.h"
#include "../Panels/ContentBrowserPanelEx.h"
#include "../UI/Views/Outliner/EntityOutlinerView.h"

namespace Relentless
{
	Editor::~Editor() noexcept {}

	const Ref<EntityOutlinerView> Editor::GetEntityOutlinerView() const noexcept
	{
		return m_pOutlinerPanel->GetEntityOutlinerView();
	}

	void Editor::OnEvent(IEvent& event) noexcept
	{
		switch (event.GetEventType())
		{
		case EventType::KeyPressedEvent:
		{
			const bool isNavigatingScene = false/* = m_HoveringSceneViewport && Mouse::IsButtonDown(RLS_Button::Right)*/;
			if (!isNavigatingScene)
			{
				const RLS_Key key = EVENT(KeyPressedEvent).key;
				switch (key)
				{
				case RLS_Key::I:
				{
					if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
						m_ImmersiveModeEnabled = !m_ImmersiveModeEnabled;

					break;
				}
				break;
				}
			}
			event.StopPropagation();
			break;
		}
		}
	}

	void Editor::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		UI_DrawMainMenuBar();

		ImGui::Begin("Content Browser");

		ImGui::End();

		ImGui::Begin("Spawn");
		
		if (ImGui::Button("Spawn viewport"))
			SpawnViewport();

		ImGui::DragFloat("Min Log Luminance", &m_MinLogLuminance, 0.1f, -100.0f, 100.0f);
		ImGui::DragFloat("Min EV100", &m_MinEV100, 0.1f, -10.0f, 20.0f);
		ImGui::DragFloat("Max EV100", &m_MaxEV100, 0.1f, -10.0f, 20.0f);
		ImGui::DragFloat("Exposure Compensation", &m_ExposureCompensation, 0.1f, -15.0f, 15.0f);

		static int counter = 0;
		if (ImGui::Button("Spawn Entity"))
		{
			m_pActiveScene->CreateEntity(std::format("Entity_{}", counter++).c_str());
		}
		if (ImGui::Button("Spawn 1000 Entities"))
		{
			for (uint32 i = 0u; i < 1000u; ++i)
				m_pActiveScene->CreateEntity(std::format("Entity_{}", counter++).c_str());
		}

		ImGui::End();
		
		ImGui::Begin("TEST");

		ImGui::Text("FPS: %d", Time::GetFramesPerSecond());

		std::sort(ProfilerManager::ProfilerMetrics.begin(), ProfilerManager::ProfilerMetrics.end(), [](const ProfilerMetrics& a, const ProfilerMetrics& b)
			{
				return a.durationInMilliSeconds > b.durationInMilliSeconds;
			});

		for (auto& p : ProfilerManager::ProfilerMetrics)
			ImGui::Text("Func: %s: MS: %f", p.ContextName, p.durationInMilliSeconds);

		ProfilerManager::ClearData();
		ImGui::End();
	}

	void Editor::OnCreate() noexcept
	{
		Project::Load(FilepathUtils::Combine(SystemPaths::GetUserHomeDirectory(), "UntitledRelentlessProject\\UntitledRelentlessProject.rproject"));

		m_pSelection = std::make_unique<Selection>();
		m_pSelection->OnSelectionChanged.Connect(this, &Editor::OnEntitySelectionChanged);

		m_pEntityFoldersManager = std::make_unique<EntityFoldersManager>(this);
		m_pEntityFoldersManager->OnEntityFolderMoved.Connect(this, &Editor::OnEntityFolderMoved);
		m_pEntityFoldersManager->OnEntityFolderDelete.Connect(this, &Editor::OnEntityFolderDeleted);

		SpawnViewport();

		SetActiveScene(std::make_shared<Scene>());
		CreateStartScene();

		m_pDetailsPanel = UIManager::Get().AddPanel(std::make_unique<DetailsPanel>(weak_from_this()));
		m_pDetailsPanel->SetPadding(Vector2(2.0f, 0.0f));

		m_pOutlinerPanel = UIManager::Get().AddPanel(std::make_unique<OutlinerPanel>(weak_from_this()));
		m_pOutlinerPanel->SetPadding(Vector2(2.0f, 0.0f));

		ContentBrowserPanelEx* pContentBrowserPanel = UIManager::Get().AddPanel(MakeUnique<ContentBrowserPanelEx>(weak_from_this()));
		pContentBrowserPanel->SetPadding(Vector2(2.0f, 0.0f));

		UIManager::Get().AddPanel(std::make_unique<TestPanel>("Test", ImGuiWindowFlags_None));

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());
		app.GetRenderer()->OnEntityIDReadbackDone.Connect(this, &Editor::OnEntityReadbackDone);
	}

	void Editor::OnDestroy() noexcept
	{
		OnShutDown();

		m_pSelection->OnSelectionChanged.Detach(this);

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());
		if (auto& pRenderer = app.GetRenderer())
			pRenderer->OnEntityIDReadbackDone.Detach(this);
	}

	void Editor::OnUpdate(const float deltaTime) noexcept
	{
		PROFILE_SCOPE("Editor::OnUpdate");

		m_pActiveScene->OnUpdate(deltaTime);

		for (int i = 0; i < m_EditorViewports.size(); ++i)
		{
			ViewportPanel* pViewportPanel = m_EditorViewports[i];

			const Vector2i& region = pViewportPanel->GetViewportSize();
			m_RenderViews[i].Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));

			const bool cameraMovementEnabled = pViewportPanel->IsClientAreaHovered();

			const ViewTransform& cameraViewTransform = pViewportPanel->GetCamera()->GetViewTransform();
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

			renderView.MouseHoverCoordinates	= pViewportPanel->IsClientAreaHovered() ? pViewportPanel->GetClientHoverCoordinates() : Vector2i(-1, -1);

			renderView.MinLogLuminance			= m_MinLogLuminance;
			renderView.MinEV100					= m_MinEV100;
			renderView.MaxEV100					= m_MaxEV100;
			renderView.ExposureCompensation		= m_ExposureCompensation;
		}
	}

	void Editor::OnRender() noexcept
	{
		//...
	}

	Scene* Editor::GetActiveScene() const noexcept
	{
		return m_pActiveScene.get();
	}

	EntityFolder* Editor::GetFolderContainingEntity(entity aEntity) const noexcept
	{
		if (!m_pActiveScene)
			return nullptr;

		return m_pActiveScene->GetEntityManager().Has<FolderComponent>(aEntity) ? m_pEntityFoldersManager->GetFolder(*m_pActiveScene, m_pActiveScene->GetEntityManager().Get<FolderComponent>(aEntity).Folder.GetPath()) : nullptr;
	}

	const UniquePtr<Selection>& Editor::GetSelection() noexcept
	{
		return m_pSelection;
	}

	const UniquePtr<EntityFoldersManager>& Editor::GetEntityFoldersManager() noexcept
	{
		return m_pEntityFoldersManager;
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

	void Editor::SetActiveScene(const std::shared_ptr<Scene>& pScene) noexcept
	{
		m_pSelection->DeselectAllEntities();

		if (m_pActiveScene)
		{
			OnPreSceneChanged(m_pActiveScene.get());

			m_pActiveScene->OnEntityPreDestroyed.Detach(this);
			m_pActiveScene->OnEntityAttached.Detach(this);
		}

		m_pActiveScene = pScene;
		m_pEditorScene = m_pActiveScene;

		m_pActiveScene->OnEntityPreDestroyed.Connect(this, &Editor::OnEntityPreDestroyed);
		m_pActiveScene->OnEntityAttached.Connect(this, &Editor::OnEntityAttached);

		OnSceneChanged(m_pActiveScene.get());
	}

	void Editor::CreateStartScene() noexcept
	{
		EntityManager& entityManager = m_pActiveScene->GetEntityManager();
		
		{
			entity dirEntity = m_pActiveScene->CreateEntity("Directional Light");
			auto& dlc = entityManager.Add<DirectionalLightComponent>(dirEntity);
			dlc.SetColor(Math::MakeFromColorTemperature(5'900.0f));
			dlc.SetIntensityLux(100'000.0f);

			auto& tc = entityManager.Get<TransformComponent>(dirEntity);
			tc.SetWorldRotationEulerDegrees(Vector3(-90.0f, 0.0f, 0.0f));
		}

		AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();

		AssetHandle whiteMaterialHandle = assetToolsModule.CreateAsset<Material>("M_DefaultWhite", "Materials/");
		Ref<Material> pWhiteMaterial = AssetManager::Get<Material>(whiteMaterialHandle);
		pWhiteMaterial->SetBlendMode(EBlendMode::Opaque);
		pWhiteMaterial->SetAlbedoColor(Colors::White);

		AssetHandle offWhiteMaterialHandle = assetToolsModule.CreateAsset<Material>("M_OffWhite", "Materials/");
		Ref<Material> pOffWhiteMaterial = AssetManager::Get<Material>(offWhiteMaterialHandle);
		pOffWhiteMaterial->SetBlendMode(EBlendMode::Opaque);
		pOffWhiteMaterial->SetAlbedoColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
		pOffWhiteMaterial->SetRoughness(1.0f);

		entity ground = m_pActiveScene->CreateEntity("Ground");
		auto& groundTC = entityManager.Get<TransformComponent>(ground);
		groundTC.AddWorldOffset(Vector3(0.0f, -1.0f, 0.0f));
		auto& mrcGround = entityManager.Add<MeshRendererComponent>(ground);
		mrcGround.AssetHandle = offWhiteMaterialHandle;

		entity sphere = m_pActiveScene->CreateEntity("Sphere");
		m_pActiveScene->AttachEntity(sphere, ground);
		auto& mrc = entityManager.Add<MeshRendererComponent>(sphere);
		mrc.AssetHandle = whiteMaterialHandle;

		auto& tc = entityManager.Get<TransformComponent>(sphere);
		tc.AddLocalOffset(Vector3::Up);

		entity topGround = m_pActiveScene->CreateEntity("TopGround");
		m_pActiveScene->AttachEntity(topGround, sphere);
		auto& mrcTopGround = entityManager.Add<MeshRendererComponent>(topGround);
		mrcTopGround.AssetHandle = whiteMaterialHandle;

		auto& tcTopGround = entityManager.Get<TransformComponent>(topGround);
		tcTopGround.AddLocalOffset(Vector3::Up);

		{
			std::vector<AssetImportTask> tasks;
			
			{
				AssetImportTask& task = tasks.emplace_back();
				task.FilePath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Models/StarterContent/Sphere.obj");

			}

			{
				AssetImportTask& task = tasks.emplace_back();
				task.FilePath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Models/StarterContent/Cube.obj");
			}

			AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
			std::vector<AssetImportResult> importResults = assetToolsModule.Import(tasks);
			
			{
				auto& mfc = m_pActiveScene->GetEntityManager().Add<MeshFilterComponent>(sphere);
				for (auto& result : importResults)
				{
					if (result.Handle.Type == Mesh::StaticType() && result.FilePath.string() == tasks[0].FilePath)
					{
						mfc.AssetHandle = result.Handle;
						break;
					}
				}

			}
			{

				auto& mfc = m_pActiveScene->GetEntityManager().Add<MeshFilterComponent>(ground);
				for (auto& result : importResults)
				{
					if (result.Handle.Type == Mesh::StaticType() && result.FilePath.string() == tasks[1].FilePath)
					{
						mfc.AssetHandle = result.Handle;
						break;
					}
				}
			}
			{
				auto& mfc = m_pActiveScene->GetEntityManager().Add<MeshFilterComponent>(topGround);
				for (auto& result : importResults)
				{
					if (result.Handle.Type == Mesh::StaticType() && result.FilePath.string() == tasks[1].FilePath)
					{
						mfc.AssetHandle = result.Handle;
						break;
					}
				}
			}

		}

		m_pEntityFoldersManager->AttachEntityToFolder(*m_pActiveScene, ground, Folder(FolderRoot::CreateFromScene(*m_pActiveScene), "StarterContent/Entities"));
	}

	void Editor::UI_DrawMainMenuBar() noexcept
	{
		if (m_ImmersiveModeEnabled)
			return;

		const float currentPadding = ImGui::GetStyle().FramePadding.y;
		const float currentBorderSize = ImGui::GetStyle().WindowBorderSize;
		ImGui::GetStyle().WindowBorderSize = 0.0f;
		ImGui::GetStyle().FramePadding.y = 10.0f;

		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(Colors::EvenRowColorDefault.R(), Colors::EvenRowColorDefault.G(), Colors::EvenRowColorDefault.B(), Colors::EvenRowColorDefault.A()));

		const bool open = ImGui::BeginMainMenuBar();
		ImGui::GetStyle().FramePadding.y = currentPadding;
		ImGui::GetStyle().WindowBorderSize = currentBorderSize;
		ImGui::PopStyleColor();

		if (!open)
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
		
		Matrix transform = pMesh->GetOffsetTransform();
		
		auto& tc = entityManager.Get<TransformComponent>(newEntity);

		Vector3 scale		= Vector3::One;
		Quaternion rotation = Quaternion::Identity;
		Vector3 location	= Vector3::Zero;

		if (!transform.Decompose(scale, rotation, location))
			return;

		tc.SetWorldScale(scale);
		tc.SetWorldRotation(rotation);
		tc.SetWorldScale(scale);
	}

	void Editor::OnEntityFolderDeleted(EntityFolder* apFolder) noexcept
	{
		if (!m_pActiveScene)
			return;

		m_pActiveScene->GetEntityManager().Collect<FolderComponent>().Do([&](entity aEntity, FolderComponent& fc)
			{
				if (fc.Folder.GetPath() == apFolder->GetPath())
					m_pActiveScene->GetEntityManager().Remove<FolderComponent>(aEntity);
			});
	}

	void Editor::OnEntityFolderMoved([[maybe_unused]] EntityFolder* pMovedFolder, [[maybe_unused]] EntityFolder* pMovedFolderParent, const String& aOldPath, const String& aNewPath) noexcept
	{
		//TODO: Possibly EntityFolders should know their root object which would link back to correct scene...

		if (!m_pActiveScene)
			return;

		m_pActiveScene->GetEntityManager().Collect<FolderComponent>().Do([&](FolderComponent& fc)
			{
				const String& path = fc.Folder.GetPath();

				if (path == aOldPath)
					fc.Folder = Folder(fc.Folder.GetRoot(), aNewPath);
				else if (path.size() > aOldPath.size() && path.compare(0, aOldPath.size(), aOldPath) == 0 && path[aOldPath.size()] == '/')
				{
					const String suffix = path.substr(aOldPath.size());
					fc.Folder = Folder(fc.Folder.GetRoot(), aNewPath + suffix);
				}
			});
	}

	void Editor::OnEntitySelectionChanged(entity e, ESelectionState selectionState)
	{
		if (selectionState == ESelectionState::Selected)
			m_pActiveScene->GetEntityManager().AddOrReplace<SelectedInEditorComponent>(e);
		else
			m_pActiveScene->GetEntityManager().Remove<SelectedInEditorComponent>(e);
	}

	void Editor::OnEntityPreDestroyed(entity aEntity) noexcept
	{
		if (m_pSelection->IsEntitySelected(aEntity))
			m_pSelection->DeselectEntity(aEntity);
	}

	void Editor::OnEntityAttached(entity aChildEntity, [[maybe_unused]] entity aParentEntity) noexcept
	{
		EntityManager& entityManager = m_pActiveScene->GetEntityManager();

		if (entityManager.Has<FolderComponent>(aChildEntity))
			entityManager.Remove<FolderComponent>(aChildEntity);
	}

	void Editor::OnEntityReadbackDone(uint32 entityID) noexcept
	{
		/*
		 An id of 0 is considered a sentinel value for the read back results.
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

	void Editor::OnViewportHotkeyPressed([[maybe_unused]] ViewportPanel* pPanel, RLS_Key key) noexcept
	{
		if (!m_pActiveScene)
			return;

		switch (key)
		{
		case RLS_Key::A:
		{
			if (!Keyboard::IsKeyDown(RLS_Key::LCtrl))
				return;
			
			m_pActiveScene->GetEntityManager().Collect<IDComponent>().Do([this](entity e)
				{
					if (!m_pSelection->IsEntitySelected(e))
						m_pSelection->SelectEntity(e);
				});
			break;
		}
		case RLS_Key::H:
		{
			SetVisibilityForSelectedEntities(Keyboard::IsKeyDown(RLS_Key::LCtrl));
			break;
		}
		case RLS_Key::Delete:
		{
			const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();
			
			for (int i = (int)selectedEntities.size() - 1; i >= 0; --i)
				m_pActiveScene->DestroyEntity(selectedEntities[i]);

			break;
		}
		}
	}

	void Editor::OnViewportClicked([[maybe_unused]] ViewportPanel* pPanel, [[maybe_unused]] Vector2u relativeMouseCoords) noexcept
	{
		const bool lCtrlDown = Keyboard::IsKeyDown(RLS_Key::LCtrl);
		const bool lShiftDown = Keyboard::IsKeyDown(RLS_Key::LShift);
		const bool isHoveringEntity = m_HoveredEntity != NULL_ENTITY;

		if (!isHoveringEntity || (!lCtrlDown && !lShiftDown))
			m_pSelection->DeselectAllEntities();

		if (isHoveringEntity)
		{
			if (lCtrlDown && m_pSelection->IsEntitySelected(m_HoveredEntity))
				m_pSelection->DeselectEntity(m_HoveredEntity);
			else
				m_pSelection->SelectEntity(m_HoveredEntity);
		}
	}

	void Editor::OnViewportEntityDuplicationRequest() noexcept
	{
		const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();
		if (selectedEntities.empty())
			return;

		std::vector<entity> newEntities;
		newEntities.reserve(selectedEntities.size());

		for (entity selectedEntity : selectedEntities)
		{
			const entity duplicatedEntity = m_pActiveScene->DuplicateEntity(selectedEntity, false);
			if (EntityFolder* pFolder = GetFolderContainingEntity(selectedEntity))
				m_pEntityFoldersManager->AttachEntityToFolder(*m_pActiveScene, duplicatedEntity, Folder(FolderRoot::CreateFromScene(*m_pActiveScene), pFolder->GetPath()));

			newEntities.push_back(duplicatedEntity);
		}

		m_pSelection->DeselectAllEntities();
		m_pSelection->SelectEntities(newEntities);
	}

	void Editor::SetVisibilityForSelectedEntities(bool aVisibilityState) noexcept
	{
		if (aVisibilityState)
		{
			m_pActiveScene->GetEntityManager().Collect<HiddenInGameComponent, RootComponent>().Do([this, aVisibilityState](entity e)
				{
					m_pActiveScene->SetEntityVisibleInGame(e, aVisibilityState);
				});

			m_pActiveScene->GetEntityManager().Collect<HiddenInGameComponent>().Do([this, aVisibilityState](entity e)
				{
					m_pActiveScene->SetEntityVisibleInGame(e, aVisibilityState);
				});
		}
		else
		{
			const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();

			for (int i = (int)selectedEntities.size() - 1; i >= 0; --i)
			{
				const entity currentEntity = selectedEntities[i];

				m_pActiveScene->SetEntityVisibleInGame(currentEntity, aVisibilityState);
				m_pSelection->DeselectEntity(currentEntity);
			}
		}
	}

	std::vector<entity> Editor::GetTransformSelection() const noexcept
	{
		PROFILE_FUNC;

		if (!m_pActiveScene || !m_pSelection)
			return {};

		const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();

		if (selectedEntities.empty())
			return {};

		std::unordered_set<entity> selectedSet;
		selectedSet.reserve(selectedEntities.size());
		for (entity e : selectedEntities)
			selectedSet.insert(e);

		std::vector<entity> participatingEntities;
		participatingEntities.reserve(selectedEntities.size());

		for (entity e : selectedEntities)
		{
			entity p = m_pActiveScene->HasParent(e) ? m_pActiveScene->GetParent(e) : NULL_ENTITY;
			bool hasSelectedAncestor = false;

			while (p != NULL_ENTITY)
			{
				if (selectedSet.find(p) != selectedSet.end())
				{
					hasSelectedAncestor = true;
					break;
				}
				p = m_pActiveScene->HasParent(p) ? m_pActiveScene->GetParent(p) : NULL_ENTITY;
			}

			if (!hasSelectedAncestor)
				participatingEntities.push_back(e);
		}

		return participatingEntities;
	}

	void Editor::SpawnViewport() noexcept
	{
		m_RenderViews.push_back(ViewportRenderView());
		
		ViewportPanel* pViewport = UIManager::Get().AddPanel(std::make_unique<ViewportPanel>(std::format("Scene Viewport {}", m_EditorViewports.size() + 1).c_str(), ImGuiWindowFlags_None, weak_from_this(), m_EditorViewports.size()));

		pViewport->OnClickedOnViewport.Connect(this, &Editor::OnViewportClicked);
		pViewport->OnHotkeyPressed.Connect(this, &Editor::OnViewportHotkeyPressed);
		pViewport->SetPadding(Vector2(2.0f, 0.0f));
		
		m_EditorViewports.push_back(pViewport);
	}

}
