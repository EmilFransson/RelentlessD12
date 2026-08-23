#include <Relentless.h>

#include "Core/Editor.h"

#include "MaterialEditorPanel.h"

#include "Subsystem/EngineContentSubsystem.h"

#include "UI/Views/Details/MaterialDetailsView.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	MaterialEditorPanel::MaterialEditorPanel(const std::vector<AssetHandle>& someMaterials) noexcept
		:SceneViewportPanel("Material Editor")
	{
		Ref<Material> pMaterial = AssetManager::Get<Material>(someMaterials.front());
		pMaterial->OnPropertyChanged.Connect(this, &MaterialEditorPanel::OnMaterialEdited);
		pMaterial->OnSaved.Connect(this, &MaterialEditorPanel::OnMaterialSaved);
		
		m_pMaterialDetailsView = RLS_NEW MaterialDetailsView(someMaterials.front());
		CreatePreviewScene();

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer) { aRenderer->CreateView(viewID); });
	}

	MaterialEditorPanel::~MaterialEditorPanel()
	{
		MaterialDetailsContext& context = m_pMaterialDetailsView->GetContext<MaterialDetailsContext>();
		if (context.Material)
		{
			context.Material->OnPropertyChanged.Detach(this);
			context.Material->OnSaved.Detach(this);
		}

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer) { aRenderer->DestroyView(viewID); });
	}

	String MaterialEditorPanel::GetDisplayName() const noexcept
	{
		MaterialDetailsContext& context = m_pMaterialDetailsView->GetContext<MaterialDetailsContext>();
		return std::format("Material Editor - {}{}", context.Material->GetName(), context.Material->IsDirty() ? "*" : "");
	}

	String MaterialEditorPanel::GetPersistKey() const noexcept
	{
		MaterialDetailsContext& context = m_pMaterialDetailsView->GetContext<MaterialDetailsContext>();
		return std::format("MaterialEditor_{}", ConvertUUIDToString(context.Material->GetUUID()));
	}

	Scene* MaterialEditorPanel::GetViewportScene() const noexcept
	{
		return m_pPreviewScene;
	}

	ViewportClient::Desc MaterialEditorPanel::CreateClientDesc()
	{
		ViewportClient::Desc desc;
		desc.Location = Vector3(0.0f, 1.0f, -2.0f);
		desc.FocusTarget = Vector3::Zero;
		desc.NavigationPreset = ECameraNavigationPreset::OrbitOnly;

		return desc;
	}

	ViewportSidePanelDesc MaterialEditorPanel::CreateSidePanelDesc()
	{
		return ViewportSidePanelDesc{ .Width = 300.0f, .StartVisible = true };
	}

	void MaterialEditorPanel::ExtendSidePanel(Ref<VerticalBox>& aSidePanelBox)
	{
		aSidePanelBox->AddWidget(m_pMaterialDetailsView);
	}

	void MaterialEditorPanel::OnInitialized()
	{
		ViewportClient& client = GetClient();
		PerspectiveCameraController* pCameraController = client.GetCameraController();
		pCameraController->SetOrbitDistance(Vector3::Distance(Vector3(0.0f, 1.0f, -2.0f), Vector3::Zero));
		pCameraController->SetMode(ECameraControllerNavigationMode::Orbit);

		client.SetRenderFeature(ERenderFeature::Grid, false);
		client.SetRenderFeature(ERenderFeature::EntityPicking, false);
		client.SetRenderFeature(ERenderFeature::Outlines, false);
		client.SetRenderFeature(ERenderFeature::ExponentialHeightFog, false);
	}

	bool MaterialEditorPanel::OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept
	{
		if (aEvent.key == RLS_Key::S && Keyboard::IsKeyDown(RLS_Key::LCtrl))
		{
			MaterialDetailsContext& context = m_pMaterialDetailsView->GetContext<MaterialDetailsContext>();
			context.Material->Save();
		}

		return true;
	}

	bool MaterialEditorPanel::OnMouseDragEvent(MouseDragEvent& aEvent) noexcept
	{
		if (Mouse::IsButtonDown(RLS_Button::Right))
		{
			ViewportClient& client = GetClient();
			PerspectiveCameraController* pCameraController = client.GetCameraController();
			PerspectiveCamera& camera = client.GetCamera();

			const float orbitDistance = pCameraController->GetOrbitDistance();
			const float delta = (aEvent.DeltaCoordinates.y * 0.005f);
			const float newOrbitDistance = Math::Max(0.1f, orbitDistance - delta);
			const float orbitDistanceDelta = newOrbitDistance - orbitDistance;

			const Vector3 location = camera.GetLocation();
			const Vector3 cameraToOrigin = Vector3::Zero - camera.GetLocation();
			Vector3 cameraToOriginDirection;
			cameraToOrigin.Normalize(cameraToOriginDirection);

			const Vector3 newLocation = location - (cameraToOriginDirection * orbitDistanceDelta);
			pCameraController->SetOrbitDistance(newOrbitDistance);
			camera.SetLocation(newLocation);
			
			return true;
		}
		else 
			return ViewportPanel::OnMouseDragEvent(aEvent);
	}

	void MaterialEditorPanel::Update() noexcept
	{
		ViewportPanel::Update();
		m_pPreviewScene->OnUpdate(Time::GetDeltaTime());
	}

	void MaterialEditorPanel::CreatePreviewScene() noexcept
	{
		m_pPreviewScene = RLS_NEW Scene("Material Preview Scene");
		EntityManager& entityManager = m_pPreviewScene->GetEntityManager();

		AssetToolsModule& assetToolsModule = ModuleManager::LoadModuleChecked<AssetToolsModule>();
		EngineContentSubsystem* pEngineContentSubsystem = Editor::Get()->GetSubsystem<EngineContentSubsystem>();
		
		const MaterialDetailsContext& materialDetailsContext = m_pMaterialDetailsView->GetContext<MaterialDetailsContext>();

		//Material Preview Entity:
		{
			m_MaterialPreviewEntity = m_pPreviewScene->CreateEntity("Material Preview");
			MeshRendererComponent& meshRendererComponent = entityManager.Add<MeshRendererComponent>(m_MaterialPreviewEntity);
			meshRendererComponent.SetMaterial(materialDetailsContext.MaterialHandle);

			MeshFilterComponent& meshFilterComponent = entityManager.Add<MeshFilterComponent>(m_MaterialPreviewEntity);
			meshFilterComponent.SetMesh(pEngineContentSubsystem->GetAssetHandle(EEngineAsset::SphereMesh));
		}

		//Floor:
		{
			const entity floor = m_pPreviewScene->CreateEntity("Floor");

			AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
			const AssetHandle floorHandle = assetTools.CreateAsset<Material>("M_FloorMaterial", "", nullptr, false);
			Ref<Material> pFloorMaterial = AssetManager::Get<Material>(floorHandle);
			pFloorMaterial->SetAlbedoColor(Color(0.5f, 0.5f, 0.5f, 1.0f));
			pFloorMaterial->SetRoughness(1.0f);
			
			MeshRendererComponent& meshRendererComponent = entityManager.Add<MeshRendererComponent>(floor);
			meshRendererComponent.SetMaterial(floorHandle);

			MeshFilterComponent& meshFilterComponent = entityManager.Add<MeshFilterComponent>(floor);
			meshFilterComponent.SetMesh(pEngineContentSubsystem->GetAssetHandle(EEngineAsset::CubeMesh));

			auto& tc = entityManager.Get<TransformComponent>(floor);
			tc.SetWorldScale(Vector3(5.0f, 0.5f, 5.0f));
			tc.SetWorldLocation(Vector3(0.0f, -0.8f, 0.0f));
		}

		//Directional Light:
		{
			entity dirLight = m_pPreviewScene->CreateLight("Directional Light", ELightType::Directional);
			auto& dlc = entityManager.Get<DirectionalLightComponent>(dirLight);
			dlc.SetColor(Math::MakeFromColorTemperature(5'900.0f));
			dlc.SetIntensityLux(150.0f);

			auto& tc = entityManager.Get<TransformComponent>(dirLight);
			tc.SetWorldRotationEulerDegrees(Vector3(-90.0f, 0.0f, 0.0f));
		}

		//Post Processing:
		{
			const entity postProcess = m_pPreviewScene->CreateEntity("Post Process");
			PostProcessVolumeComponent& postProcessComponent = entityManager.Add<PostProcessVolumeComponent>(postProcess);
			ExposureSettings& exposureSettings = postProcessComponent.GetExposure();
			exposureSettings.SetHistogramMinEV100(0.0f);
			exposureSettings.SetHistogramMaxEV100(0.0f);

			exposureSettings.SetMinEV100(5.0f);
			exposureSettings.SetMaxEV100(5.0f);
		}

		const AssetHandle environmentHandle = assetToolsModule.CreateAsset<Environment>("MaterialPreviewEnvironment", "", nullptr, false);
		Ref<Environment> pMaterialPreviewEnvironment = AssetManager::Get<Environment>(environmentHandle);
		pMaterialPreviewEnvironment->SetSourceType(EEnvironmentSourceType::Cubemap);
		pMaterialPreviewEnvironment->SetEnvironmentMapHandle(pEngineContentSubsystem->GetAssetHandle(EEngineAsset::QuattroCantiTextureCube));

		//Sky Box:
		{
			const entity skyBox = m_pPreviewScene->CreateEntity("Sky Box");
			SkyBoxComponent& skyBoxComponent = entityManager.Add<SkyBoxComponent>(skyBox);
			skyBoxComponent.SetPrimaryEnvironment(environmentHandle);

			m_pPreviewScene->SetActiveSkyBox(skyBox);
		}

		//Sky Light:
		{
			const entity skyLight = m_pPreviewScene->CreateEntity("Sky Light");
			SkyLightComponent& skyLightComponent = entityManager.Add<SkyLightComponent>(skyLight);
			skyLightComponent.SetPrimaryEnvironment(environmentHandle);
			skyLightComponent.SetLowerHemisphereMode(ESkyLightLowerHemisphereMode::SolidColor);
			skyLightComponent.SetLowerHemisphereColor(Colors::White);

			m_pPreviewScene->SetActiveSkyLight(skyLight);
		}
	}

	void MaterialEditorPanel::OnMaterialEdited(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		RebuildName();
	}

	void MaterialEditorPanel::OnMaterialSaved(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RebuildName();
	}
}
