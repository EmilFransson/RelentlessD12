#include "EnvironmentEditorPanel.h"

#include "Core/Editor.h"

#include "Scene/Scene.h"

#include "UI/DragDrop/AssetDragDropOperation.h"
#include "UI/Views/Details/EnvironmentDetailsView.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	EnvironmentEditorPanel::EnvironmentEditorPanel(const std::vector<AssetHandle>& someEnvironments) noexcept
		:ViewportPanel("Environment Editor")
	{
		m_pEnvironmentDetailsView = RLS_NEW EnvironmentDetailsView(someEnvironments.front());
		CreatePreviewScene();

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer)
			{ 
				aRenderer->CreateView(viewID);
			});
		
		Ref<Environment> pEnvironment = AssetManager::Get<Environment>(someEnvironments.front());
		pEnvironment->OnPropertyChanged.Connect(this, &EnvironmentEditorPanel::OnEnvironmentEdited);
		pEnvironment->OnSaved.Connect(this, &EnvironmentEditorPanel::OnEnvironmentSaved);

		SetRoot(BuildWindowLayout());
	}

	EnvironmentEditorPanel::~EnvironmentEditorPanel()
	{
		EnvironmentDetailsContext& context = m_pEnvironmentDetailsView->GetContext<EnvironmentDetailsContext>();
		if (context.Environment)
		{
			context.Environment->OnPropertyChanged.Detach(this);
			context.Environment->OnSaved.Detach(this);
		}

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer)
			{ 
				aRenderer->DestroyView(viewID);
			});
	}

	ViewRenderDesc EnvironmentEditorPanel::BuildRenderDescriptor() const noexcept
	{
		const SharedPtr<PerspectiveCamera> pCamera = GetCamera();

		RenderFeatures renderFeatures{};
		renderFeatures.Disable(ERenderFeature::Grid);
		renderFeatures.Disable(ERenderFeature::HBAOPlus);
		renderFeatures.Disable(ERenderFeature::EntityPicking);
		renderFeatures.Disable(ERenderFeature::Outlines);

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

	Ref<VerticalBox> EnvironmentEditorPanel::BuildWindowLayout() noexcept
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

		pDetailsViewBox->AddWidget(m_pEnvironmentDetailsView);

		m_pCanvas->OnDragOver(this, &EnvironmentEditorPanel::OnCanvasDragOver);
		m_pCanvas->OnDrop(this, &EnvironmentEditorPanel::OnDropOnCanvas);

		return pRoot;
	}

	String EnvironmentEditorPanel::GetDisplayName() const noexcept
	{
		EnvironmentDetailsContext& context = m_pEnvironmentDetailsView->GetContext<EnvironmentDetailsContext>();
		return std::format("Environment Editor - {}{}", context.Environment->GetName(), context.Environment->IsDirty() ? "*" : "");
	}

	String EnvironmentEditorPanel::GetPersistKey() const noexcept
	{
		EnvironmentDetailsContext& context = m_pEnvironmentDetailsView->GetContext<EnvironmentDetailsContext>();
		return std::format("EnvironmentEditor_{}", ConvertUUIDToString(context.Environment->GetUUID()));
	}

	bool EnvironmentEditorPanel::OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept
	{
		if (aEvent.key == RLS_Key::S && Keyboard::IsKeyDown(RLS_Key::LCtrl))
		{
			EnvironmentDetailsContext& context = m_pEnvironmentDetailsView->GetContext<EnvironmentDetailsContext>();
			context.Environment->Save();
			return true;
		}

		return ViewportPanel::OnKeyPressedEvent(aEvent);
	}

	void EnvironmentEditorPanel::Update() noexcept
	{
		ViewportPanel::Update();
		m_pPreviewScene->OnUpdate(Time::GetDeltaTime());
	}

	void EnvironmentEditorPanel::CreatePreviewScene() noexcept
	{
		m_pPreviewScene = RLS_NEW Scene("Environment Preview Scene");
		const EnvironmentDetailsContext& environmentDetailsContext = m_pEnvironmentDetailsView->GetContext<EnvironmentDetailsContext>(); 

		m_EnvironmentPreviewEntity = m_pPreviewScene->CreateEntity("Environment Preview");
		SkyBoxComponent& skyBoxComponent = m_pPreviewScene->GetEntityManager().Add<SkyBoxComponent>(m_EnvironmentPreviewEntity);
		skyBoxComponent.SetPrimaryEnvironment(environmentDetailsContext.EnvironmentHandle);

		m_pPreviewScene->SetActiveSkyBox(m_EnvironmentPreviewEntity);
	}

	Reply EnvironmentEditorPanel::OnCanvasDragOver(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetDragDropOperation>())
			return Reply::Unhandled();

		AssetDragDropOperation& assetDragDropOperation = aDragDropOperation->AsType<AssetDragDropOperation>();
		if (assetDragDropOperation.GetNumDraggedAssets() != 1u)
			return Reply::Unhandled();

		if (assetDragDropOperation.GetAssets().front().Type != Environment::StaticType())
			return Reply::Unhandled();

		return Reply::Handled();
	}

	Reply EnvironmentEditorPanel::OnDropOnCanvas(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetDragDropOperation>())
			return Reply::Unhandled();

		AssetDragDropOperation& assetDragDropOperation = aDragDropOperation->AsType<AssetDragDropOperation>();
		if (assetDragDropOperation.GetNumDraggedAssets() != 1u)
			return Reply::Unhandled();

		const AssetData& assetData = assetDragDropOperation.GetAssets().front();
		if (assetData.Type != Environment::StaticType())
			return Reply::Unhandled();

		const AssetHandle environmentHandle = AssetManager::LoadAsset(assetData);
		if (!environmentHandle.IsValid())
			return Reply::Unhandled();

		if (m_pEnvironmentDetailsView->GetContext<EnvironmentDetailsContext>().EnvironmentHandle.Uuid == environmentHandle.Uuid)
			return Reply::Unhandled();

		SkyBoxComponent& skyBoxComponent = m_pPreviewScene->GetEntityManager().Get<SkyBoxComponent>(m_EnvironmentPreviewEntity);
		skyBoxComponent.SetPrimaryEnvironment(environmentHandle);
		m_pEnvironmentDetailsView->SetEnvironment(environmentHandle);

		return Reply::Handled();
	}

	void EnvironmentEditorPanel::OnEnvironmentEdited(MAYBE_UNUSED IAsset* aAsset, MAYBE_UNUSED uint64 aProperty) noexcept
	{
		RebuildName();
	}

	void EnvironmentEditorPanel::OnEnvironmentSaved(MAYBE_UNUSED IAsset* aAsset) noexcept
	{
		RebuildName();
	}

}
