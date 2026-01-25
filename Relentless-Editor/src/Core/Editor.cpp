#include "Editor.h"
#include "RelentlessEditorApp.h"

#include "../Assets/Factory/MaterialFactory.h"
#include "../Assets/Factory/MeshFactory.h"
#include "../Assets/Factory/ModelFactory.h"
#include "../Assets/Factory/TextureFactory.h"
#include "../Module/ContentBrowserModule.h"
#include "../Module/UIModule.h"
#include "../UI/Views/Outliner/EntityOutlinerView.h"

#include "../Subsystem/SelectionSubsystem.h"
#include "../Subsystem/EntityFoldersSubsystem.h"
#include "../Subsystem/EditorSceneBridgeSubsystem.h"

namespace Relentless
{
	Editor::~Editor() noexcept {}

	//Lazy loads at fetch time:
	void Editor::CreateSubsystems() noexcept
	{
		//Note: Load order should be preserved:
		GetSubsystem<SelectionSubsystem>();
		GetSubsystem<EntityFoldersSubsystem>();
		GetSubsystem<EditorSceneBridgeSubsystem>();
	}

	const Ref<EntityOutlinerView> Editor::GetEntityOutlinerView() const noexcept
	{
		return m_pOutlinerPanel->GetEntityOutlinerView();
	}

	void Editor::OnEvent(IEvent& event) noexcept
	{
		{
			std::lock_guard<std::mutex> lock(m_OnEventMutex);
			for (const auto& [type, callBack] : m_EventCallbacks)
			{
				if (callBack(event))
					return;
			}
		}

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
				default:
					break;
				}
			}
			event.StopPropagation();
			break;
		}
		default:
			break;
		}
	}

	void Editor::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		UI_DrawMainMenuBar();

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

		{
			std::lock_guard<std::mutex> lock(m_OnUIRenderMutex);
			for (const auto& [id, callBack] : m_UIRenderCallbacks)
				callBack();
		}
	}

	void Editor::OnCreate() noexcept
	{
		LoadModules();
		Project::Load("D:\\UntitledRelentlessProject\\UntitledRelentlessProject.rproject");
		CreateSubsystems();
		CreateStartScene();
		LoadBrdfLut_Temp();
		SpawnViewport();
	}

	void Editor::OnDestroy() noexcept
	{
		OnShutDown();

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());
		if (auto& pRenderer = app.GetRenderer())
			pRenderer->OnEntityIDReadbackDone.Detach(this);
	}

	void Editor::OnUpdate(const float deltaTime) noexcept
	{
		PROFILE_SCOPE("Editor::OnUpdate");

		m_pActiveScene->OnUpdate(deltaTime);

		{
			std::lock_guard<std::mutex> lock(m_OnUpdateMutex);
			for (const auto& [id, callBack] : m_UpdateCallbacks)
				callBack(deltaTime);
		}

		for (size_t i = 0; i < m_EditorViewports.size(); ++i)
		{
			ViewportPanel* pViewportPanel = m_EditorViewports[i];

			const Vector2i& region = pViewportPanel->GetViewportSize();
			m_RenderViews[i].Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));

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

	CallbackID Editor::RegisterEventCallback(Callback<bool(IEvent&)> aEventCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnEventMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_EventCallbacks.emplace(toReturn, std::move(aEventCallback));

		return toReturn;
	}

	CallbackID Editor::RegisterUpdateCallback(Callback<void(float)> aUpdateCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUpdateMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_UpdateCallbacks.emplace(toReturn, std::move(aUpdateCallback));

		return toReturn;
	}

	CallbackID Editor::RegisterUIRenderCallback(Callback<void()> aUpdateCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUIRenderMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_UIRenderCallbacks.emplace(toReturn, std::move(aUpdateCallback));
		return toReturn;
	}

	void Editor::UnregisterEventCallback(CallbackID aCallbackHandle) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnEventMutex);
		m_UIRenderCallbacks.erase(aCallbackHandle);
	}

	void Editor::UnregisterUpdateCallback(CallbackID aCallbackHandle) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUpdateMutex);
		m_UpdateCallbacks.erase(aCallbackHandle);
	}

	void Editor::UnregisterUIRenderCallback(CallbackID aCallbackHandle) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnUIRenderMutex);
		m_UIRenderCallbacks.erase(aCallbackHandle);
	}

	Scene* Editor::GetActiveScene() const noexcept
	{
		return m_pActiveScene.Get();
	}

	EntityFolder* Editor::GetFolderContainingEntity(entity aEntity) const noexcept
	{
		if (!m_pActiveScene)
			return nullptr;

		return m_pActiveScene->GetEntityManager().Has<FolderComponent>(aEntity) ? GetSubsystem<EntityFoldersSubsystem>()->GetFolder(*m_pActiveScene, m_pActiveScene->GetEntityManager().Get<FolderComponent>(aEntity).Folder.GetPath()) : nullptr;
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

	void Editor::SetActiveScene(const Ref<Scene>& aScene) noexcept
	{
		if (m_pActiveScene)
			OnSceneChange(m_pActiveScene.Get());

		m_pActiveScene = aScene;
		m_pEditorScene = m_pActiveScene;

		OnSceneChanged(m_pActiveScene.Get());
	}

	void Editor::CreateStartScene() noexcept
	{
		SetActiveScene(new Scene());

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();

		while (assetRegistry.IsLoadingAssets()){}

		EntityManager& entityManager = m_pActiveScene->GetEntityManager();
		
		{
			entity dirEntity = m_pActiveScene->CreateLight("Directional Light", ELightType::Directional);
			auto& dlc = entityManager.Get<DirectionalLightComponent>(dirEntity);
			dlc.SetColor(Math::MakeFromColorTemperature(5'900.0f));
			dlc.SetIntensityLux(100'000.0f);

			auto& tc = entityManager.Get<TransformComponent>(dirEntity);
			tc.SetWorldRotationEulerDegrees(Vector3(-90.0f, 0.0f, 0.0f));
		}

		AssetHandle offWhiteMaterialHandle = AssetManager::LoadAsset("Assets/Materials/M_OffWhite");

		Ref<Material> pOffWhiteMaterial = AssetManager::Get<Material>(offWhiteMaterialHandle);
		pOffWhiteMaterial->SetBlendMode(EBlendMode::Opaque);
		pOffWhiteMaterial->SetAlbedoColor(Vector4(0.5f, 0.5f, 0.5f, 1.0f));
		pOffWhiteMaterial->SetRoughness(1.0f);

		entity cubeEntity = m_pActiveScene->CreateEntity("Cube");
		EntityManager& mgr = m_pActiveScene->GetEntityManager();

		AssetHandle meshHandle = AssetManager::LoadAsset("Assets/Meshes/Cube");
		mgr.Add<MeshFilterComponent>(cubeEntity).AssetHandle = meshHandle;
		mgr.Add<MeshRendererComponent>(cubeEntity).AssetHandle = offWhiteMaterialHandle;
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

	void Editor::LoadBrdfLut_Temp() noexcept
	{
		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());

		Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
		pFactory->SetImportAsSRGB(false);
		pFactory->SetGenerateMipmaps(false);

		std::vector<AssetImportTask> tasks;
		AssetImportTask& task = tasks.emplace_back();
		task.FilePath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), "Assets/Textures/brdf_ibl_lut.dds");
		task.pFactory = pFactory;
		task.DestinationPath = "Engine/Textures/";

		AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		m_BRDFLutTextureHandle = assetToolsModule.Import(tasks)[0].Handle;

		const auto& renderer = app.GetRenderer();
		renderer->OnEntityIDReadbackDone.Connect(this, &Editor::OnEntityReadbackDone);
		renderer->OnRequestBRDFLut(this, &Editor::OnRequestBRDFLut);
	}

	void Editor::LoadModules() noexcept
	{
		//Note: Load order should be preserved:
		AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		assetTools.RegisterFactory<Material>(RLS_NEW MaterialFactory());
		assetTools.RegisterFactory<Mesh>(RLS_NEW MeshFactory());
		assetTools.RegisterFactory<Texture2D>(RLS_NEW TextureFactory());
		assetTools.RegisterCompositeFactory<ModelFactory>(RLS_NEW ModelFactory());

		ModuleManager::LoadModuleChecked<UIModule>();
		ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		ModuleManager::LoadModuleChecked<AssetRegistryModule>();
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

	AssetHandle Editor::OnRequestBRDFLut() noexcept
	{
		return m_BRDFLutTextureHandle;
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
					SelectionSubsystem* pSelection = GetSubsystem<SelectionSubsystem>();

					if (!pSelection->IsEntitySelected(e))
						pSelection->SelectEntity(e);
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
			SelectionSubsystem* pSelection = GetSubsystem<SelectionSubsystem>();
			const std::vector<entity>& selectedEntities = pSelection->GetSelectedEntities();
			
			for (int i = (int)selectedEntities.size() - 1; i >= 0; --i)
				m_pActiveScene->DestroyEntity(selectedEntities[i]);

			break;
		}
		default:
			break;
		}
	}

	void Editor::OnViewportClicked([[maybe_unused]] ViewportPanel* pPanel, [[maybe_unused]] Vector2u relativeMouseCoords) noexcept
	{
		const bool lCtrlDown = Keyboard::IsKeyDown(RLS_Key::LCtrl);
		const bool lShiftDown = Keyboard::IsKeyDown(RLS_Key::LShift);
		const bool isHoveringEntity = m_HoveredEntity != NULL_ENTITY;

		SelectionSubsystem* pSelection = GetSubsystem<SelectionSubsystem>();

		if (!isHoveringEntity || (!lCtrlDown && !lShiftDown))
			pSelection->DeselectAllEntities();

		if (isHoveringEntity)
		{
			if (lCtrlDown && pSelection->IsEntitySelected(m_HoveredEntity))
				pSelection->DeselectEntity(m_HoveredEntity);
			else
				pSelection->SelectEntity(m_HoveredEntity);
		}
	}

	void Editor::OnViewportEntityDuplicationRequest() noexcept
	{
		SelectionSubsystem* pSelection = GetSubsystem<SelectionSubsystem>();

		const std::vector<entity>& selectedEntities = pSelection->GetSelectedEntities();
		if (selectedEntities.empty())
			return;

		std::vector<entity> newEntities;
		newEntities.reserve(selectedEntities.size());

		for (entity selectedEntity : selectedEntities)
		{
			const entity duplicatedEntity = m_pActiveScene->DuplicateEntity(selectedEntity, false);
			if (EntityFolder* pFolder = GetFolderContainingEntity(selectedEntity))
				GetSubsystem<EntityFoldersSubsystem>()->AttachEntityToFolder(*m_pActiveScene, duplicatedEntity, Folder(FolderRoot::CreateFromScene(*m_pActiveScene), pFolder->GetPath()));

			newEntities.push_back(duplicatedEntity);
		}

		pSelection->DeselectAllEntities();
		pSelection->SelectEntities(newEntities);
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
			SelectionSubsystem* pSelection = GetSubsystem<SelectionSubsystem>();

			const std::vector<entity>& selectedEntities = pSelection->GetSelectedEntities();

			for (int i = (int)selectedEntities.size() - 1; i >= 0; --i)
			{
				const entity currentEntity = selectedEntities[i];

				m_pActiveScene->SetEntityVisibleInGame(currentEntity, aVisibilityState);
				pSelection->DeselectEntity(currentEntity);
			}
		}
	}

	std::vector<entity> Editor::GetTransformSelection() const noexcept
	{
		PROFILE_FUNC;

		if (!m_pActiveScene)
			return {};

		const SelectionSubsystem* pSelection = GetSubsystem<SelectionSubsystem>();

		const std::vector<entity>& selectedEntities = pSelection->GetSelectedEntities();

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
		
		ViewportPanel* pViewport = ModuleManager::LoadModuleChecked<UIModule>().AddPanel<ViewportPanel>(m_EditorViewports.size());
		pViewport->OnClickedOnViewport.Connect(this, &Editor::OnViewportClicked);
		pViewport->OnHotkeyPressed.Connect(this, &Editor::OnViewportHotkeyPressed);
		
		m_EditorViewports.push_back(pViewport);
	}

}
