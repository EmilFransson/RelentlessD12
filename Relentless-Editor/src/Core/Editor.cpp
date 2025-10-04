#include "Editor.h"
#include "RelentlessEditorApp.h"

namespace Relentless
{
	Editor::~Editor() noexcept {}

	bool Editor::AnyFolderContainsEntity(entity aEntity) const noexcept
	{
		return m_pActiveScene && m_pActiveScene->GetEntityManager().Exists(aEntity) && m_pActiveScene->GetEntityManager().Has<FolderComponent>(aEntity);
	}

	void Editor::AttachEntityToFolder(entity aEntity, const Folder& aFolder) noexcept
	{
		if (!m_pActiveScene)
			return;

		if (m_pActiveScene->EntityHasAncestors(aEntity))
			m_pActiveScene->DetachEntity(aEntity);
		
		if (!m_pEntityFoldersManager->ContainsFolder(*m_pActiveScene, aFolder))
			m_pEntityFoldersManager->CreateFolder(*m_pActiveScene, aFolder);

		m_pActiveScene->GetEntityManager().AddOrReplace<FolderComponent>(aEntity).Folder = aFolder;
		OnEntityAttachedToFolder(aEntity, aFolder);
	}

	bool Editor::FolderContainsEntity(const Folder& aFolder, entity aEntity) noexcept
	{
		return m_pActiveScene && m_pActiveScene->GetEntityManager().Has<FolderComponent>(aEntity) && m_pActiveScene->GetEntityManager().Get<FolderComponent>(aEntity).Folder == aFolder;
	}

	const Ref<EntityOutlinerView> Editor::GetEntityOutlinerView() const noexcept
	{
		return m_pDetailsPanel->GetEntityOutlinerView();
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
				case RLS_Key::A:
				{
					//if (m_pOutlinerPanel->IsFocused())
					//	m_pOutlinerPanel->SelectAllExpanded();

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

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Main", nullptr);

		ImGui::BeginChild("Scene", ImVec2(0,60), false);

		ImGui::EndChild();

		ImGuiID dockspaceID = ImGui::GetID("MainDockSpace");
		ImGui::DockSpace(dockspaceID, ImVec2(0, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::Begin("Content Browser");

		ImGui::End();

		ImGui::Begin("Spawn");
		
		if (ImGui::Button("Spawn viewport"))
			SpawnViewport();
		
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
		

		////UI_DrawSceneStateIcons();
	
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
		//
		//UI_DrawStatisticsPanel();
		//
		//m_pOutlinerPanel->OnImGuiRender(m_DisplayOutlinerPanel && !m_ImmersiveModeEnabled);
		//m_InspectorPanel.OnImGuiRender(m_DisplayInspectorPanel && !m_ImmersiveModeEnabled);
		//m_SceneRendererPanel.OnImGuiRender(m_DisplaySceneRendererPanel && !m_ImmersiveModeEnabled);
		//m_PropertiesPanel.OnImGuiRender(m_DisplayPropertiesPanel && !m_ImmersiveModeEnabled);
		//m_ContentBrowserPanel.OnImGuiRender(m_DisplayContentBrowserPanel && !m_ImmersiveModeEnabled);
		//m_MetricsPanel.OnImGuiRender(m_DisplayMetricsPanel && !m_ImmersiveModeEnabled);

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
		//TODO: Change ffs.
		//UI::Initialize();

		m_pSelection = std::make_unique<Selection>();
		m_pSelection->OnSelectionChanged.Connect(this, &Editor::OnEntitySelectionChanged);

		m_pEntityFoldersManager = std::make_unique<EntityFoldersManager>(this);
		m_pEntityFoldersManager->OnEntityFolderMoved.Connect(this, &Editor::OnEntityFolderMoved);
		m_pEntityFoldersManager->OnEntityFolderDelete.Connect(this, &Editor::OnEntityFolderDeleted);

		SpawnViewport();

		//m_pOutlinerPanel = std::make_unique<OutlinerPanel>(this);
		//m_pOutlinerPanel = UIManager::Get().AddPanel(std::make_unique<OutlinerPanel>(this));

		SetActiveScene(std::make_shared<Scene>());
		
		CreateStartScene();

		m_pDetailsPanel = UIManager::Get().AddPanel(std::make_unique<DetailsPanel>(ICON_FA_LINES_LEANING " Details", ImGuiWindowFlags_None, this));
		m_pDetailsPanel->SetPadding(Vector2(2.0f, 0.0f));

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());
		app.GetRenderer()->OnEntityIDReadbackDone.Connect(this, &Editor::OnEntityReadbackDone);

		//LoadStarterMeshes();
		
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
		OnShutDown();

		m_pSelection->OnSelectionChanged.Detach(this);

		RelentlessEditor& app = static_cast<RelentlessEditor&>(Application::Get());
		if (auto& pRenderer = app.GetRenderer())
		{
			pRenderer->OnEntityIDReadbackDone.Detach(this);
		}
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

			renderView.MouseHoverCoordinates = pViewportPanel->IsClientAreaHovered() ? pViewportPanel->GetClientHoverCoordinates() : Vector2i(-1, -1);
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
		//{
		//	Ref<ModelFactory> pFactory = new ModelFactory();
		//	pFactory->OnDone.Connect([this](const std::vector<ImportedAsset>& assets, bool success)
		//		{
		//			RLS_VERIFY(success, "[Editor::CreateStartScene] Failed to import Sponza.");
		//
		//			EntityManager& entityManager = m_pActiveScene->GetEntityManager();
		//
		//			for (auto& asset : assets)
		//			{
		//				if (asset.Type != AssetType::Mesh)
		//					continue;
		//
		//				Ref<Mesh> pMesh = AssetManager::Get<Mesh>(asset.Handle);
		//				const entity newEntity = m_pActiveScene->CreateEntity(pMesh->GetName().c_str());
		//
		//				MeshRendererComponent& mrc = entityManager.Add<MeshRendererComponent>(newEntity);
		//				mrc.AssetHandle = pMesh->GetDefaultMaterialHandle();
		//				RLS_VERIFY(mrc.AssetHandle != NULL_HANDLE, "[Editor::CreateStartScene] AssetHandle Is invalid.");
		//
		//				MeshFilterComponent& mfc = entityManager.Add<MeshFilterComponent>(newEntity);
		//				mfc.AssetHandle = asset.Handle;
		//
		//				const Transform& transform = pMesh->GetOffsetTransform();
		//
		//				auto& tc = entityManager.Get<TransformComponent>(newEntity);
		//				m_pActiveScene->SetWorldTransform(newEntity, transform.Matrix);
		//			}
		//		});
		//
		//	std::vector<AssetImportTask> tasks;
		//	AssetImportTask& task = tasks.emplace_back();
		//	task.FilePath = "C:/Users/emilf/Downloads/main_sponza/Main.1_Sponza/NewSponza_Main_glTF_002.gltf";
		//	task.pFactory = pFactory;
		//
		//	Importer::RequestAsyncLoad(tasks).Wait();
		//}

		entity dirEntity = m_pActiveScene->CreateEntity("Directional Light");
		auto& dlc = m_pActiveScene->GetEntityManager().Add<DirectionalLightComponent>(dirEntity);
		dlc.Color = Math::MakeFromColorTemperature(5'900.0f);
		dlc.Intensity = Math::LuxToRadiantIrradiance(100'000.0f);

		Ref<Material> pWhiteMaterial = new Material();
		pWhiteMaterial->SetBlendMode(EBlendMode::Opaque);
		pWhiteMaterial->m_AlbedoColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		pWhiteMaterial->m_Metallic = 0.0f;
		pWhiteMaterial->m_Roughness = 0.5f;

		const uint32 index = AssetManager::GetStorage<Material>().Add(pWhiteMaterial);
		auto [handle, _] = AssetManager::InsertMetaData(CreateUUID(), index, AssetType::Material);

		Ref<Material> pNewWhiteMaterial = new Material();
		pNewWhiteMaterial->SetBlendMode(EBlendMode::Opaque);
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

		m_pActiveScene->AttachEntity(e, ground);

		m_pActiveScene->GetEntityManager().Add<RotatorComponent>(e);

		auto& mrc = m_pActiveScene->GetEntityManager().Add<MeshRendererComponent>(e);
		mrc.AssetHandle = handle->second;

		{
			std::vector<AssetImportTask> tasks;

			auto&& CreateTextureImportTask = [&tasks, this](const Path& relativePath, entity entity, bool srgb, auto SetTextureFunc)
				{
					Ref<TextureFactory> pFactory = RLS_NEW TextureFactory();
					pFactory->SetImportAsSRGB(srgb);

					pFactory->OnDone.Connect([this, entity, SetTextureFunc](const ImportedAsset& asset, bool success)
						{
							RLS_VERIFY(success, "[Editor::OnCreate] Asset import failed.");

							auto& mrc = m_pActiveScene->GetEntityManager().Get<MeshRendererComponent>(entity);
							Ref<Material> pMaterial = AssetManager::Get<Material>(mrc.AssetHandle);
							(pMaterial.Get()->*SetTextureFunc)(asset.Handle);
						});

					AssetImportTask& task = tasks.emplace_back();
					task.FilePath = FilepathUtils::Combine(EDITOR_ASSET_DIRECTORY, relativePath);
					task.pFactory = pFactory;
				};

			std::mutex entityManagerAccessMutex;
			auto&& CreateModelImportTask = [&tasks, this, &entityManagerAccessMutex](const Path& relativePath, entity entity)
				{
					Ref<ModelFactory> pFactory = RLS_NEW ModelFactory();
					pFactory->OnDone.Connect([this, entity, &entityManagerAccessMutex](const std::vector<ImportedAsset>& assets, bool success)
						{
							RLS_VERIFY(success, "[Editor::OnCreate] Asset import failed.");

							std::lock_guard guard(entityManagerAccessMutex);
							auto& mfc = m_pActiveScene->GetEntityManager().Add<MeshFilterComponent>(entity);

							for (auto& asset : assets)
							{
								if (asset.Type == AssetType::Mesh)
								{
									mfc.AssetHandle = asset.Handle;
									break;
								}
							}
						});

					AssetImportTask& task = tasks.emplace_back();
					task.FilePath = FilepathUtils::Combine(FilePath::GetEngineWorkingDirectory(), relativePath);
					task.pFactory = pFactory;
				};

			CreateModelImportTask("Assets/Models/StarterContent/Sphere.obj", e);
			CreateModelImportTask("Assets/Models/StarterContent/Cube.obj", ground);

			//CreateTextureImportTask("Textures/rusty-metal-ue/rusty-metal_albedo.png", e, true, &Material::SetAlbedoTexture);
			//CreateTextureImportTask("Textures/rusty-metal-ue/rusty-metal_metallic.png", e, false, &Material::SetMetallicTexture);
			//CreateTextureImportTask("Textures/rusty-metal-ue/rusty-metal_roughness.png", e, false, &Material::SetRoughnessTexture);
			//CreateTextureImportTask("Textures/rusty-metal-ue/rusty-metal_ao.png", e, false, &Material::SetAmbientOcclusionTexture);
			//CreateTextureImportTask("Textures/rusty-metal-ue/rusty-metal_normal-dx.png", e, false, &Material::SetNormalMap);
			//CreateTextureImportTask("Textures/rusty-metal-ue/rusty-metal_height.png", e, false, &Material::SetHeightMap);

			Importer::RequestAsyncLoad(tasks).Wait();
		}

		//const entity parent = m_pActiveScene->CreateShape(Shape::Cube);
		//const entity child = m_pActiveScene->CreateShape(Shape::Torus);
		//const entity otherCube = m_pActiveScene->CreateShape(Shape::Capsule);
		//const entity other = m_pActiveScene->CreateShape(Shape::Sphere);
		//m_pActiveScene->SetWorldLocation(parent, DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f));
		//m_pActiveScene->SetWorldLocation(child, DirectX::XMFLOAT3(5.0f, 0.0f, 0.0f));
		//m_pActiveScene->SetWorldLocation(otherCube, DirectX::XMFLOAT3(-5.0f, 0.0f, 0.0f));
		//m_pActiveScene->SetWorldLocation(other, DirectX::XMFLOAT3(-8.0f, 0.0f, 0.0f));
		//
		
		//RunFolderComprehensiveTest();

		AttachEntityToFolder(ground, Folder(FolderRoot::CreateFromScene(*m_pActiveScene), "StarterContent/Entities"));
	}

	// Helper: quick starts_with for String
	static bool StartsWith(const String& s, std::string_view prefix) {
		return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
	}

	static bool ConsoleIsUTF8()
	{
#ifdef _WIN32
		return GetConsoleOutputCP() == 65001; // CP_UTF8
#else
		return true; // most non-Windows terminals are UTF-8
#endif
	}
	struct TreeGlyphs { const char* elbow; const char* tee; const char* pipe; const char* space; };

	void Editor::RunFolderComprehensiveTest()
	{
		RLS_ASSERT(m_pActiveScene && m_pEntityFoldersManager, "Active scene or folders manager missing");
		Scene& scene = *m_pActiveScene;
		auto& F = *m_pEntityFoldersManager;
		const FolderRoot root = FolderRoot::CreateFromScene(scene);

		auto dump = [&](std::string_view tag)
			{
				const TreeGlyphs g = ConsoleIsUTF8()
					? TreeGlyphs{ "└─ ", "├─ ", "│  ", "   " }   // UTF-8 bytes in source
				: TreeGlyphs{ "\\- ", "|- ", "|  ", "   " }; // ASCII fallback

				std::cout << "\n=== " << tag << " ===\n";

				// Gather folders
				auto& cont = F.GetFolderContainer(scene).Folders;
				std::vector<EntityFolder*> nodes; nodes.reserve(cont.size());
				for (auto& rf : cont) nodes.push_back(rf);

				// Build children map by parent*
				std::unordered_map<EntityFolder*, std::vector<EntityFolder*>> children;
				for (auto* n : nodes) children[n->GetParent()].push_back(n);
				for (auto& [p, ch] : children)
					std::sort(ch.begin(), ch.end(),
						[](auto* a, auto* b) { return a->GetLabel() < b->GetLabel(); });

				// Tree print
				std::function<void(EntityFolder*, std::string, bool)> dfs =
					[&](EntityFolder* n, std::string indent, bool last)
					{
						std::cout << indent << (last ? g.elbow : g.tee) << n->GetPath() << "\n";
						auto it = children.find(n);
						if (it == children.end()) return;
						auto& ch = it->second;
						for (size_t i = 0; i < ch.size(); ++i)
							dfs(ch[i], indent + (last ? g.space : g.pipe), i + 1 == ch.size());
					};

				std::cout << "[Folders :: tree]\n";
				if (auto it = children.find(nullptr); it != children.end()) {
					auto& roots = it->second;
					for (size_t i = 0; i < roots.size(); ++i)
						dfs(roots[i], "", i + 1 == roots.size());
				}
				else {
					std::cout << "(no folders)\n";
				}

				// Flat list + duplicates
				std::vector<EntityFolder*> sorted = nodes;
				std::sort(sorted.begin(), sorted.end(),
					[](auto* a, auto* b) { return a->GetPath() < b->GetPath(); });

				std::unordered_map<String, int> pathCount;
				for (auto* n : sorted) ++pathCount[n->GetPath()];

				std::cout << "\n[Folders :: flat]\n";
				for (auto* n : sorted) {
					const auto* p = n->GetParent();
					const bool dup = pathCount[n->GetPath()] > 1;
					std::cout << " - " << n->GetPath()
						<< "   (label=\"" << n->GetLabel() << "\""
						<< ", parent=" << (p ? p->GetPath() : "<null>")
						<< (dup ? ", DUPLICATE" : "") << ")\n";
				}

				// Entities
				struct Row { entity e; String name; String path; };
				std::vector<Row> rows;
				scene.GetEntityManager().Collect<FolderComponent>().Do(
					[&](entity e, const FolderComponent& fc)
					{
						rows.push_back({ e,
										 scene.GetEntityManager().Get<NameComponent>(e).Name,
										 fc.Folder.GetPath() });
					});
				std::sort(rows.begin(), rows.end(),
					[](const Row& a, const Row& b) {
						if (a.path != b.path) return a.path < b.path;
						return a.name < b.name;
					});

				std::cout << "\n[Entities]\n";
				for (const auto& r : rows)
					std::cout << " - " << r.path << "   " << r.name
					<< " (e=" << (uint64_t)r.e << ")\n";
			};

		// Clean start (optional): ensure no pre-existing conflicting folders/entities
		// You can clear your scene here if you have a helper.

		F.CreateFolder(scene, "Root/A/B");
		F.CreateFolder(scene, "Root/B/A/A");
		entity e = scene.CreateEntity("X");
		AttachEntityToFolder(e, Folder(root, "Root/B/A/A"));

		dump("before delete Root/B");
		F.DeleteFolder(scene, "Root/B");
		dump("after delete Root/B");

		RLS_ASSERT(scene.GetEntityManager().Get<FolderComponent>(e).Folder.GetPath() == "Root/A/A", "Should be merged under Root/A/A");

		// ─────────────────────────────────────────────────────────────────────────────
		// 1) Create chain and idempotency
		// ─────────────────────────────────────────────────────────────────────────────
		{
			EntityFolder* a = F.CreateFolder(scene, "A");
			RLS_ASSERT(a && a->GetLabel() == "A", "Create A failed");

			EntityFolder* ab = F.CreateFolder(scene, "A/B");
			RLS_ASSERT(ab && ab->GetLabel() == "B" && ab->GetParent() == a, "Create A/B failed");

			// Idempotent create: returns the same object and keeps correct parent
			EntityFolder* ab2 = F.CreateFolder(scene, "A/B");
			RLS_ASSERT(ab2 == ab, "Idempotent CreateFolder returned a different object");
			RLS_ASSERT(ab2->GetParent() == a, "Idempotent CreateFolder changed parent incorrectly");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 2) Default name uniqueness
		// ─────────────────────────────────────────────────────────────────────────────
		{
			String p = F.GetDefaultFolderName(scene, "A");
			RLS_ASSERT(p == "A/New Folder", "Expected A/New Folder");

			F.CreateFolder(scene, p); // create it

			String p2 = F.GetDefaultFolderName(scene, "A");
			RLS_ASSERT(p2 == "A/New Folder2", "Expected A/New Folder2");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 3) Boundary correctness: "Foo" vs "FooBar"
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "Foo");
			F.CreateFolder(scene, "FooBar"); // must not be affected by renaming Foo

			const entity eFoo = scene.CreateEntity("InFoo");
			const entity eFooBar = scene.CreateEntity("InFooBar");

			AttachEntityToFolder(eFoo, Folder(root, "Foo"));
			AttachEntityToFolder(eFooBar, Folder(root, "FooBar"));

			// Rename Foo -> Baz
			bool ok = F.RenameFolder(scene, "Foo", "Baz");
			RLS_ASSERT(ok, "Rename Foo->Baz failed");

			const auto& pFoo = scene.GetEntityManager().Get<FolderComponent>(eFoo).Folder.GetPath();
			const auto& pFooB = scene.GetEntityManager().Get<FolderComponent>(eFooBar).Folder.GetPath();
			RLS_ASSERT(pFoo == "Baz", "Entity in Foo not remapped to Baz");
			RLS_ASSERT(pFooB == "FooBar", "Entity in FooBar should not be affected");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 4) Deep subtree rename + entity remap
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "StarterContent/Entities/Monsters");
			F.CreateFolder(scene, "StarterContent/Entities/Monsters/Bosses");
			F.CreateFolder(scene, "StarterContent/Props");

			const entity ground = scene.CreateEntity("Ground");
			const entity troll = scene.CreateEntity("TrollBoss");
			const entity crate = scene.CreateEntity("Crate");

			AttachEntityToFolder(ground, Folder(root, "StarterContent/Entities/Monsters"));
			AttachEntityToFolder(troll, Folder(root, "StarterContent/Entities/Monsters/Bosses"));
			AttachEntityToFolder(crate, Folder(root, "StarterContent/Props"));

			dump("before deep rename");

			// Monsters -> Enemies
			bool ok = F.RenameFolder(scene,
				"StarterContent/Entities/Monsters",
				"StarterContent/Entities/Enemies");
			RLS_ASSERT(ok, "Rename Monsters->Enemies failed");

			dump("after deep rename");

			const auto& groundPath = scene.GetEntityManager().Get<FolderComponent>(ground).Folder.GetPath();
			const auto& trollPath = scene.GetEntityManager().Get<FolderComponent>(troll).Folder.GetPath();
			const auto& cratePath = scene.GetEntityManager().Get<FolderComponent>(crate).Folder.GetPath();

			RLS_ASSERT(groundPath == "StarterContent/Entities/Enemies", "Remap failed for ground");
			RLS_ASSERT(trollPath == "StarterContent/Entities/Enemies/Bosses", "Remap failed for troll");
			RLS_ASSERT(cratePath == "StarterContent/Props", "Unrelated entity changed unexpectedly");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 5) Invalid rename: into own descendant
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "X/Y");
			F.CreateFolder(scene, "X/Y/Z");

			bool ok = F.RenameFolder(scene, "X/Y", "X/Y/Z"); // invalid
			RLS_ASSERT(!ok, "Should not allow rename into own descendant");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 6) Rename to an existing path (should fail, no changes)
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "Alpha");
			F.CreateFolder(scene, "Beta");

			// Put an entity in Alpha to detect unintended changes
			const entity a = scene.CreateEntity("A");
			AttachEntityToFolder(a, Folder(root, "Alpha"));

			bool ok = F.RenameFolder(scene, "Alpha", "Beta");
			RLS_ASSERT(!ok, "Rename to existing folder should fail");

			const auto& pathA = scene.GetEntityManager().Get<FolderComponent>(a).Folder.GetPath();
			RLS_ASSERT(pathA == "Alpha", "Entity path changed despite failed rename");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 7) Delete middle parent: children re-parent and entities remap away from prefix
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "SC/Entities/Enemies/Minions");
			const entity m1 = scene.CreateEntity("Minion1");
			const entity m2 = scene.CreateEntity("Minion2");

			AttachEntityToFolder(m1, Folder(root, "SC/Entities/Enemies"));
			AttachEntityToFolder(m2, Folder(root, "SC/Entities/Enemies/Minions"));

			dump("before delete SC/Entities");

			// Delete middle parent "SC/Entities"
			F.DeleteFolder(scene, "SC/Entities");

			dump("after delete SC/Entities");

			const auto& mp1 = scene.GetEntityManager().Get<FolderComponent>(m1).Folder.GetPath();
			const auto& mp2 = scene.GetEntityManager().Get<FolderComponent>(m2).Folder.GetPath();

			// Policy-agnostic check: old prefix must be gone
			RLS_ASSERT(!StartsWith(mp1, "SC/Entities/"), "m1 still has deleted prefix");
			RLS_ASSERT(!StartsWith(mp2, "SC/Entities/"), "m2 still has deleted prefix");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 8) ForEachEntityInFolders: selection + early break
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "Pick/One");
			F.CreateFolder(scene, "Pick/Two");

			const entity e1 = scene.CreateEntity("Pick1");
			const entity e2 = scene.CreateEntity("Pick2");
			const entity e3 = scene.CreateEntity("Pick3");

			AttachEntityToFolder(e1, Folder(root, "Pick/One"));
			AttachEntityToFolder(e2, Folder(root, "Pick/Two"));
			AttachEntityToFolder(e3, Folder(root, "Pick/Two"));

			std::unordered_set<String> paths = { "Pick/One", "Pick/Two" };

			std::vector<entity> visited;
			m_pEntityFoldersManager->ForEachEntityInFolders(scene, paths,
				[&](entity e)
				{
					visited.push_back(e);
					// Early break after 2
					return visited.size() < 2;
				});

			RLS_ASSERT(visited.size() == 2, "Early-break in ForEachEntityInFolders failed");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 9) Expanded state flag round-trip
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "UI/Windows");
			RLS_ASSERT(F.IsFolderExpanded(scene, "UI/Windows"), "Expected collapsed by default");

			if (Ref<EntityFolder> f = F.GetFolder(scene, "UI/Windows"))
				f->SetExpandedState(false);

			RLS_ASSERT(!F.IsFolderExpanded(scene, "UI/Windows"), "Expanded state not persisted");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 10) Root-level rename (no parent)
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "Top");
			const entity t = scene.CreateEntity("TopGuy");
			AttachEntityToFolder(t, Folder(root, "Top"));

			bool ok = F.RenameFolder(scene, "Top", "TopRenamed");
			RLS_ASSERT(ok, "Root-level rename failed");

			const auto& tp = scene.GetEntityManager().Get<FolderComponent>(t).Folder.GetPath();
			RLS_ASSERT(tp == "TopRenamed", "Root-level entity not remapped");
		}

		// ─────────────────────────────────────────────────────────────────────────────
		// 11) Delete leaf, no crash, entity policy respected (no old prefix)
		// ─────────────────────────────────────────────────────────────────────────────
		{
			F.CreateFolder(scene, "Leaf");
			const entity l = scene.CreateEntity("Leafy");
			AttachEntityToFolder(l, Folder(root, "Leaf"));

			F.DeleteFolder(scene, "Leaf");
			bool has = scene.GetEntityManager().Has<FolderComponent>(l);
			RLS_ASSERT(!has, "Leaf entity still references deleted folder");
		}

		std::cout << "\nAll folder tests passed ✅\n";
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
			if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
			{
				m_pActiveScene->GetEntityManager().Collect<HiddenInGameComponent, RootComponent>().Do([this](entity e)
					{
						m_pActiveScene->SetEntityVisibleInGame(e, true);
					});
			}
			else
			{
				const std::vector<entity>& selectedEntities = m_pSelection->GetSelectedEntities();

				for (int i = (int)selectedEntities.size() - 1; i >= 0; --i)
				{
					const entity currentEntity = selectedEntities[i];

					m_pActiveScene->SetEntityVisibleInGame(currentEntity, false);
					m_pSelection->DeselectEntity(currentEntity);
				}
			}
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
		{
			//m_pOutlinerPanel->DeselectNonEntityItems();
			m_pSelection->DeselectAllEntities();
		}

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
			newEntities.push_back(m_pActiveScene->DuplicateEntity(selectedEntity, false));

		m_pSelection->DeselectAllEntities();
		m_pSelection->SelectEntities(newEntities);
	}

	void Editor::RemoveEntityFromCurrentFolder(entity aEntity) noexcept
	{
		if (AnyFolderContainsEntity(aEntity))
		{
			Folder folder = m_pActiveScene->GetEntityManager().Get<FolderComponent>(aEntity).Folder;
			m_pActiveScene->GetEntityManager().Remove<FolderComponent>(aEntity);
			OnEntityRemovedFromFolder(aEntity, folder);
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
		
		ViewportPanel* pViewport = UIManager::Get().AddPanel(std::make_unique<ViewportPanel>(std::format("Scene Viewport {}", m_EditorViewports.size() + 1).c_str(), ImGuiWindowFlags_None, this, m_EditorViewports.size()));

		pViewport->OnClickedOnViewport.Connect(this, &Editor::OnViewportClicked);
		pViewport->OnHotkeyPressed.Connect(this, &Editor::OnViewportHotkeyPressed);
		pViewport->SetPadding(Vector2(2.0f, 0.0f));
		
		m_EditorViewports.push_back(pViewport);
	}

}
