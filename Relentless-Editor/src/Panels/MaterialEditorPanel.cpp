#include <Relentless.h>

#include "Core/Editor.h"

#include "MaterialEditorPanel.h"

#include "Subsystem/EngineContentSubsystem.h"

#include "UI/Views/Details/MaterialDetailsView.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	MaterialEditorPanel::MaterialEditorPanel(const std::vector<AssetHandle>& someMaterials) noexcept
		:ViewportPanel("Material Editor")
	{
		Ref<Material> pMaterial = AssetManager::Get<Material>(someMaterials.front());
		pMaterial->OnPropertyChanged.Connect(this, &MaterialEditorPanel::OnMaterialEdited);
		pMaterial->OnSaved.Connect(this, &MaterialEditorPanel::OnMaterialSaved);
		
		m_pMaterialDetailsView = RLS_NEW MaterialDetailsView(someMaterials.front());
		CreatePreviewScene();

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer)
			{
				aRenderer->CreateView(viewID);
			});

		SetRoot(BuildWindowLayout());
	}

	MaterialEditorPanel::~MaterialEditorPanel()
	{
		MaterialDetailsContext& context = m_pMaterialDetailsView->GetContext<MaterialDetailsContext>();
		if (context.Material)
		{
			context.Material->OnPropertyChanged.Detach(this);
			context.Material->OnSaved.Detach(this);
		}

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer)
			{
				aRenderer->DestroyView(viewID);
			});
	}

	ViewRenderDesc MaterialEditorPanel::BuildRenderDescriptor() const noexcept
	{
		const SharedPtr<PerspectiveCamera> pCamera = GetCamera();

		RenderFeatures renderFeatures{};
		renderFeatures.Disable(ERenderFeature::Grid);
		renderFeatures.Disable(ERenderFeature::EntityPicking);

		RenderQualitySettings renderQualitySettings{};

		ViewRenderDesc renderDesc
		{
			.ViewTransform = pCamera->GetViewTransform(),
			.SceneID = m_pPreviewScene->GetUUID(),
			.ViewID = GetUUID(),
			.RenderFeatures = renderFeatures,
			.RenderQualitySettings = renderQualitySettings,
			.MouseHoverCoordinates = IsClientAreaHovered() ? GetClientHoverCoordinates() : Vector2i(-1, -1),
			.RenderTarget = m_pRenderTarget
		};

		const Vector2i& region = GetViewportSize();
		renderDesc.ViewTransform.Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));

		return renderDesc;
	}

	Ref<VerticalBox> MaterialEditorPanel::BuildWindowLayout() noexcept
	{
		Ref<VerticalBox> pRoot = RLS_NEW VerticalBox();
		pRoot->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);

		Ref<HorizontalBox> pBox = pRoot->AddWidget(RLS_NEW HorizontalBox());
		pBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		pBox->AddWidget(BuildDefaultCanvasWidget());

		Ref<HorizontalBox> pDetailsViewBox = pBox->AddWidget(RLS_NEW HorizontalBox());
		pDetailsViewBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		pDetailsViewBox->SetSize(Vector2(300.0f, -1.0f));
		pDetailsViewBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		pDetailsViewBox->AddWidget(m_pMaterialDetailsView);

		return pRoot;
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
			const UniquePtr<PerspectiveCameraController>& pCameraController = GetCameraController();
			const float orbitDistance = pCameraController->GetOrbitDistance();
			pCameraController->SetOrbitDistance(orbitDistance - (aEvent.DeltaCoordinates.y * 0.005f));
			return true;
		}
		else 
			return ViewportPanel::OnMouseDragEvent(aEvent);
	}

	void MaterialEditorPanel::ResolveAndSetCameraMode() noexcept
	{
		//Nothing - Only orbit allowed.
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
			meshFilterComponent.SetMesh(pEngineContentSubsystem->GetSphereMeshHandle());
		}

		//Floor:
		{
			const entity floor = m_pPreviewScene->CreateEntity("Floor");

			AssetToolsModule& assetTools = ModuleManager::LoadModuleChecked<AssetToolsModule>();
			const AssetHandle floorHandle = assetTools.CreateAsset<Material>("M_FloorMaterial", "", nullptr, false);
			Ref<Material> pFloorMaterial = AssetManager::Get<Material>(floorHandle);
			pFloorMaterial->SetAlbedoColor(Color(0.01f, 0.01f, 0.01f, 1.0f));
			pFloorMaterial->SetRoughness(1.0f);
			
			MeshRendererComponent& meshRendererComponent = entityManager.Add<MeshRendererComponent>(floor);
			meshRendererComponent.SetMaterial(floorHandle);

			MeshFilterComponent& meshFilterComponent = entityManager.Add<MeshFilterComponent>(floor);
			meshFilterComponent.SetMesh(pEngineContentSubsystem->GetCubeMeshHandle());

			auto& tc = entityManager.Get<TransformComponent>(floor);
			tc.SetWorldScale(Vector3(5.0f, 0.5f, 5.0f));
			tc.SetWorldLocation(Vector3(0.0f, -0.8f, 0.0f));
		}

		//Directional Light:
		{
			entity dirLight = m_pPreviewScene->CreateLight("Directional Light", ELightType::Directional);
			auto& dlc = entityManager.Get<DirectionalLightComponent>(dirLight);
			dlc.SetColor(Math::MakeFromColorTemperature(5'900.0f));
			dlc.SetIntensityLux(2'000.0f);

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

			exposureSettings.SetMinEV100(3.0f);
			exposureSettings.SetMaxEV100(3.0f);
		}

		const AssetHandle environmentHandle = assetToolsModule.CreateAsset<Environment>("MaterialPreviewEnvironment", "", nullptr, false);
		Ref<Environment> pMaterialPreviewEnvironment = AssetManager::Get<Environment>(environmentHandle);
		pMaterialPreviewEnvironment->SetSourceType(EEnvironmentSourceType::Cubemap);
		pMaterialPreviewEnvironment->SetEnvironmentMapHandle(pEngineContentSubsystem->GetMaterialPreviewCubemapHandle());

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
		
		//Camera:
		{
			constexpr Vector3 location = Vector3(0.0f, 1.0f, -2.0f);
			Quaternion rotation = Math::CreateLookToRotation(location, Vector3::Zero);
			rotation.Normalize();

			SharedPtr<PerspectiveCamera> pCamera = GetCamera();
			pCamera->SetLocation(location);
			pCamera->SetRotation(rotation);

			const UniquePtr<PerspectiveCameraController>& pCameraController = GetCameraController();
			pCameraController->SetOrbitDistance(Vector3::Distance(location, Vector3::Zero));
			pCameraController->SetMode(ECameraControllerNavigationMode::Orbit);
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
