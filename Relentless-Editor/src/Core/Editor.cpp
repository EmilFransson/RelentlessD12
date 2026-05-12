#include "Editor.h"
#include "RelentlessEditorApp.h"

#include "Assets/Factory/EnvironmentFactory.h"
#include "Assets/Factory/MaterialFactory.h"
#include "Assets/Factory/MeshFactory.h"
#include "Assets/Factory/ModelFactory.h"
#include "Assets/Factory/TextureFactory.h"

#include "ECS/Components/MeshFilterComponent.h"
#include "ECS/Components/MeshRendererComponent.h"

#include "Module/ContentBrowserModule.h"
#include "Module/DetailsModule.h"
#include "Module/UIModule.h"

#include "Panels/EditorViewportPanel.h"
#include "Panels/OutlinerPanel.h"

#include "Subsystem/SelectionSubsystem.h"
#include "Subsystem/EntityFoldersSubsystem.h"
#include "Subsystem/EditorSceneBridgeSubsystem.h"
#include "Subsystem/EditorRendererBridgeSubsystem.h"
#include "Subsystem/EditorViewportSubsystem.h"
#include "Subsystem/EngineContentSubsystem.h"

#include "UI/Views/Outliner/EntityOutlinerView.h"

namespace Relentless
{
	Editor::~Editor() noexcept {}

	//Lazy loads at fetch time:
	void Editor::CreateSubsystems() noexcept
	{
		Application& app = Application::Get();

		//Note: Load order should be preserved:
		EngineContentSubsystem* pContentSubsystem = GetSubsystem<EngineContentSubsystem>();
		while (pContentSubsystem->IsLoading())
		{
			app.FlushMainThreadQueue();
		}

		GetSubsystem<SelectionSubsystem>();
		GetSubsystem<EntityFoldersSubsystem>();
		GetSubsystem<EditorSceneBridgeSubsystem>();
		GetSubsystem<EditorRendererBridgeSubsystem>();
	}

	const Ref<EntityOutlinerView> Editor::GetEntityOutlinerView() const noexcept
	{
		RLS_ASSERT(false, "TODO!");
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
	}

	void Editor::OnImGuiRender() noexcept
	{
		PROFILE_FUNC;

		UI_DrawMainMenuBar();

		ImGui::Begin("Spawn");
		
		if (ImGui::Button("Spawn viewport"))
			ModuleManager::LoadModuleChecked<UIModule>().OpenPanel<EditorViewportPanel>();

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
		GetSubsystem<EditorViewportSubsystem>();
		LoadModules();
		Project::Load("D:\\UntitledRelentlessProject\\UntitledRelentlessProject.rproject");
		CreateSubsystems();
		CreateStartScene();
	}

	void Editor::OnDestroy() noexcept
	{
		ModuleManager::ShutDown();
		OnShutDown();
	}

	void Editor::OnUpdate(float aDeltaTime) noexcept
	{
		PROFILE_SCOPE("Editor::OnUpdate");

		if (m_pActiveScene)
			m_pActiveScene->OnUpdate(aDeltaTime);
		
		UpdateSubsystems(aDeltaTime);
	}

	void Editor::OnRender() noexcept
	{
	}

	CallbackID Editor::RegisterEventCallback(Callback<bool(IEvent&)> aEventCallback) noexcept
	{
		std::lock_guard<std::mutex> lock(m_OnEventMutex);
		static CallbackID nextCallbackID = 0;
		CallbackID toReturn = nextCallbackID++;

		m_EventCallbacks.emplace(toReturn, std::move(aEventCallback));

		return toReturn;
	}

	CallbackID Editor::RegisterUpdateCallback(Callback<void(float)> aUpdateCallback, float aUpdateRate) noexcept
	{
		UpdateCallbackContext context
		{
			.Callback = std::move(aUpdateCallback),
			.UpdateRate = aUpdateRate,
			.AccumulatedTime = 0.0f,
			.Alive = true
		};

		CallbackID index = 0u;
		if (!m_UpdateCallbacksFreeList.empty())
		{
			index = m_UpdateCallbacksFreeList.front();
			m_UpdateCallbacksFreeList.pop();
		}
		else
			index = static_cast<CallbackID>(m_UpdateCallbacks.size());

		if (index == m_UpdateCallbacks.size())
			m_UpdateCallbacks.push_back(std::move(context));
		else
			m_UpdateCallbacks[index] = std::move(context);

		return index;
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
		m_EventCallbacks.erase(aCallbackHandle);
	}

	void Editor::UnregisterUpdateCallback(CallbackID aCallbackHandle) noexcept
	{
		RLS_ASSERT(aCallbackHandle < static_cast<CallbackID>(m_UpdateCallbacks.size()), "[Editor::UnregisterUpdateCallback]: Callback handle is invalid.");
		m_UpdateCallbacks[aCallbackHandle].Alive = false;
		m_UpdateCallbacksFreeList.push(aCallbackHandle);
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
		SetActiveScene(RLS_NEW Scene());

		AssetRegistryModule& assetRegistry = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		while (assetRegistry.IsLoadingAssets()) {}

		EngineContentSubsystem* pContentSubsystem = GetSubsystem<EngineContentSubsystem>();
		EntityManager& entityManager = m_pActiveScene->GetEntityManager();

		{
			entity dirEntity = m_pActiveScene->CreateLight("Point Light", ELightType::Directional);
			auto& dlc = entityManager.Get<DirectionalLightComponent>(dirEntity);
			dlc.SetColor(Math::MakeFromColorTemperature(5'900.0f));
			dlc.SetIntensityLux(2'000.0f);

			auto& tc = entityManager.Get<TransformComponent>(dirEntity);
			tc.SetWorldRotationEulerDegrees(Vector3(-90.0f, 0.0f, 0.0f));
		}

		AssetHandle offWhiteMaterialHandle = AssetManager::LoadAsset("Game/Materials/OffWhite");
		AssetHandle meshHandle = pContentSubsystem->GetCubeMeshHandle();

		//Opaque stuff:
		for (uint32 i = 0u; i < 10u; ++i)
		{
			entity cubeEntity = m_pActiveScene->CreateEntity(std::format("Cube_{}", i).c_str());
			entityManager.Add<MeshFilterComponent>(cubeEntity).SetMesh(meshHandle);
			entityManager.Add<MeshRendererComponent>(cubeEntity).SetMaterial(offWhiteMaterialHandle);
			entityManager.Get<TransformComponent>(cubeEntity).AddWorldOffset(Vector3((float)i, 0.0f, 0.0f));
		}

		AssetHandle groundMaterialHandle = AssetManager::LoadAsset("Game/Materials/Ground");
		AssetHandle sphereMeshHandle = pContentSubsystem->GetSphereMeshHandle();

		//Transparent stuff:
		for (uint32 i = 10; i < 20u; ++i)
		{
			entity cubeEntity = m_pActiveScene->CreateEntity(std::format("Cube_{}", i).c_str());
			entityManager.Add<MeshFilterComponent>(cubeEntity).SetMesh(sphereMeshHandle);
			entityManager.Add<MeshRendererComponent>(cubeEntity).SetMaterial(groundMaterialHandle);
			entityManager.Get<TransformComponent>(cubeEntity).AddWorldOffset(Vector3((float)i, 0.0f, 0.0f));
		}

		const entity postProcessEntity = m_pActiveScene->CreateEntity("Post Process");
		entityManager.Add<PostProcessVolumeComponent>(postProcessEntity);

		const entity skyBoxAndLightEntity = m_pActiveScene->CreateEntity("SkyBoxAndLight");
		m_pActiveScene->GetEntityManager().Add<SkyLightComponent>(skyBoxAndLightEntity);
		m_pActiveScene->GetEntityManager().Add<SkyBoxComponent>(skyBoxAndLightEntity);

		m_pActiveScene->SetActiveSkyLight(skyBoxAndLightEntity);
		m_pActiveScene->SetActiveSkyBox(skyBoxAndLightEntity);

		AssetManager::LoadAssetAsync("Game/Environments/PureSky/PureSkyEnvironment", [this, skyBoxAndLightEntity](const AssetHandle& aHandle)
			{
				RLS_ASSERT(aHandle.IsValid(), "PureSky environment is invalid.");
				m_pActiveScene->GetEntityManager().Get<SkyLightComponent>(skyBoxAndLightEntity).SetPrimaryEnvironment(aHandle);
				m_pActiveScene->GetEntityManager().Get<SkyBoxComponent>(skyBoxAndLightEntity).SetPrimaryEnvironment(aHandle);
			});

		AssetManager::LoadAssetAsync("Game/Environments/Overcast/OvercastEnvironment", [this, skyBoxAndLightEntity](const AssetHandle& aHandle)
			{
				RLS_ASSERT(aHandle.IsValid(), "Overcast environment is invalid.");
				m_pActiveScene->GetEntityManager().Get<SkyLightComponent>(skyBoxAndLightEntity).SetBlendEnvironment(aHandle);
				m_pActiveScene->GetEntityManager().Get<SkyBoxComponent>(skyBoxAndLightEntity).SetBlendEnvironment(aHandle);
			});
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

	void Editor::LoadModules() noexcept
	{
		//Note: Load order should be preserved:
		AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		assetTools.RegisterFactory<Material>(RLS_NEW MaterialFactory());
		assetTools.RegisterFactory<Mesh>(RLS_NEW MeshFactory());
		assetTools.RegisterFactory<Texture2D>(RLS_NEW TextureFactory());
		assetTools.RegisterFactory<TextureCube>(RLS_NEW TextureFactory());
		assetTools.RegisterFactory<Environment>(RLS_NEW EnvironmentFactory());
		assetTools.RegisterCompositeFactory<ModelFactory>(RLS_NEW ModelFactory());

		ModuleManager::LoadModuleChecked<DetailsModule>();
		ModuleManager::LoadModuleChecked<UIModule>().OpenPanel<EditorViewportPanel>();
		ModuleManager::LoadModuleChecked<ContentBrowserModule>();
		
		AssetRegistryModule& assetRegistryModule = ModuleManager::LoadModuleChecked<AssetRegistryModule>();
		assetRegistryModule.RegisterRoot(SystemPaths::GetEditorAssetsDirectory(), EAssetSourceType::Engine);
		assetRegistryModule.ScanForAssets(SystemPaths::GetEditorAssetsDirectory());
		
		while (assetRegistryModule.IsLoadingAssets()) {}

		ModuleManager::LoadModuleChecked<RenderModule>();
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

	void Editor::UpdateSubsystems(float aDeltaTime) noexcept
	{
		for (UpdateCallbackContext& callbackContext : m_UpdateCallbacks)
		{
			if (!callbackContext.Alive)
				continue;

			if (callbackContext.UpdateRate <= 0.0f)
			{
				callbackContext.Callback(aDeltaTime);
				continue;
			}

			callbackContext.AccumulatedTime += aDeltaTime;

			if (callbackContext.AccumulatedTime >= callbackContext.UpdateRate)
			{
				callbackContext.Callback(callbackContext.AccumulatedTime);
				callbackContext.AccumulatedTime = 0.0f;
			}
		}
	}
}
