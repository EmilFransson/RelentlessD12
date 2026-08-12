#include "EnvironmentEditorPanel.h"

#include "Scene/Scene.h"

#include "UI/DragDrop/AssetViewDragDropOperation.h"
#include "UI/Views/Details/EnvironmentDetailsView.h"
#include "UI/Widgets/Canvas.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	EnvironmentEditorPanel::EnvironmentEditorPanel(const std::vector<AssetHandle>& someEnvironments) noexcept
		: SceneViewportPanel("Environment Editor")
	{
		m_pEnvironmentDetailsView = RLS_NEW EnvironmentDetailsView(someEnvironments.front());
		CreatePreviewScene();

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer) { aRenderer->CreateView(viewID); });
		
		Ref<Environment> pEnvironment = AssetManager::Get<Environment>(someEnvironments.front());
		pEnvironment->OnPropertyChanged.Connect(this, &EnvironmentEditorPanel::OnEnvironmentEdited);
		pEnvironment->OnSaved.Connect(this, &EnvironmentEditorPanel::OnEnvironmentSaved);
	}

	EnvironmentEditorPanel::~EnvironmentEditorPanel()
	{
		EnvironmentDetailsContext& context = m_pEnvironmentDetailsView->GetContext<EnvironmentDetailsContext>();
		if (context.Environment)
		{
			context.Environment->OnPropertyChanged.Detach(this);
			context.Environment->OnSaved.Detach(this);
		}

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer) {  aRenderer->DestroyView(viewID); });
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

	Scene* EnvironmentEditorPanel::GetViewportScene() const noexcept
	{
		return m_pPreviewScene;
	}

	ViewportSidePanelDesc EnvironmentEditorPanel::CreateSidePanelDesc()
	{
		return ViewportSidePanelDesc{ .Width = 300.0f, .StartVisible = true };
	}

	void EnvironmentEditorPanel::ExtendSidePanel(Ref<VerticalBox>& aSidePanelBox)
	{
		aSidePanelBox->AddWidget(m_pEnvironmentDetailsView);
	}

	void EnvironmentEditorPanel::OnInitialized()
	{
		Canvas* pCanvas = GetCanvas();
		pCanvas->OnDragOver(this, &EnvironmentEditorPanel::OnCanvasDragOver);
		pCanvas->OnDrop(this, &EnvironmentEditorPanel::OnDropOnCanvas);

		ViewportClient& client = GetClient();
		client.SetRenderFeature(ERenderFeature::Grid, false);
		client.SetRenderFeature(ERenderFeature::HBAOPlus, false);
		client.SetRenderFeature(ERenderFeature::EntityPicking, false);
		client.SetRenderFeature(ERenderFeature::Outlines, false);
		client.SetRenderFeature(ERenderFeature::ExponentialHeightFog, false);
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
		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return Reply::Unhandled();

		AssetViewDragDropOperation& assetViewDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
		if (assetViewDragDropOperation.GetNumDraggedAssets() != 1u)
			return Reply::Unhandled();

		if (assetViewDragDropOperation.GetAssets().front().Type != Environment::StaticType())
			return Reply::Unhandled();

		return Reply::Handled();
	}

	Reply EnvironmentEditorPanel::OnDropOnCanvas(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		if (!aDragDropOperation->IsOfType<AssetViewDragDropOperation>())
			return Reply::Unhandled();

		AssetViewDragDropOperation& assetDragDropOperation = aDragDropOperation->AsType<AssetViewDragDropOperation>();
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
