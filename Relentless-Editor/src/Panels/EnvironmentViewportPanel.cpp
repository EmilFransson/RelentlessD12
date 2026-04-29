#include "EnvironmentViewportPanel.h"

#include "Core/Editor.h"

#include "Scene/Scene.h"

#include "UI/Views/Details/EnvironmentDetailsView.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	EnvironmentViewportPanel::EnvironmentViewportPanel(const std::vector<Ref<Environment>>& someEnvironments) noexcept
		:ViewportPanel("Environment Editor")
	{
		m_pEnvironmentDetailsView = RLS_NEW EnvironmentDetailsView(someEnvironments);
		m_pPreviewScene = RLS_NEW Scene("Environment Preview Scene");

		Renderer::Dispatch([sceneID = m_pPreviewScene->GetUUID(), viewID = GetUUID()](Renderer* aRenderer)
			{ 
				aRenderer->CreateRenderScene(sceneID);
				aRenderer->CreateView(viewID);
			});

		SetRoot(BuildWindowLayout());
	}

	EnvironmentViewportPanel::~EnvironmentViewportPanel()
	{
		Renderer::Dispatch([sceneID = m_pPreviewScene->GetUUID(), viewID = GetUUID()](Renderer* aRenderer)
			{ 
				aRenderer->DestroyRenderScene(sceneID);
				aRenderer->DestroyView(viewID);
			});
	}

	ViewRenderDesc EnvironmentViewportPanel::BuildRenderDescriptor() const noexcept
	{
		const SharedPtr<PerspectiveCamera> pCamera = GetCamera();

		RenderFeatures renderFeatures;
		renderFeatures.Disable(ERenderFeature::Grid);
		renderFeatures.Disable(ERenderFeature::HBAOPlus);
		renderFeatures.Disable(ERenderFeature::EntityPicking);
		
		Scene* pScene = Editor::Get()->GetActiveScene();

		ViewRenderDesc renderDesc
		{
			.ViewTransform = pCamera->GetViewTransform(),
			.SceneID = pScene->GetUUID(),
			.ViewID = GetUUID(),
			.RenderFeatures = renderFeatures,
			.RenderQualitySettings = {},
			.MouseHoverCoordinates = IsClientAreaHovered() ? GetClientHoverCoordinates() : Vector2i(-1, -1),
			.RenderTarget = m_pRenderTarget,
			.Scene = pScene
		};

		const Vector2i& region = GetViewportSize();
		renderDesc.ViewTransform.Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));

		return renderDesc;
	}

	Ref<VerticalBox> EnvironmentViewportPanel::BuildWindowLayout() noexcept
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

		return pRoot;
	}

	void EnvironmentViewportPanel::Update() noexcept
	{
		ViewportPanel::Update();
		m_pPreviewScene->OnUpdate(Time::GetDeltaTime());
	}

}
