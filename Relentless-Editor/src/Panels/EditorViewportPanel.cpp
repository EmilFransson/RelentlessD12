#include "EditorViewportPanel.h"

#include "Core/Editor.h"

#include "Graphics/Renderer/Renderer.h"

#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	EditorViewportPanel::EditorViewportPanel() noexcept
		:ViewportPanel("Scene Viewport")
	{
		SetRoot(BuildDefaultWindowLayout());

		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer)
			{
				aRenderer->CreateView(viewID);
			});
	}

	EditorViewportPanel::~EditorViewportPanel() noexcept
	{
		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer)
			{
				aRenderer->DestroyView(viewID);
			});
	}

	ViewRenderDesc EditorViewportPanel::BuildRenderDescriptor() const noexcept
	{
		const SharedPtr<PerspectiveCamera> pCamera = GetCamera();
		Editor* pEditor = Editor::Get();

		RenderQualitySettings renderQualitySettings;
		renderQualitySettings.MSAASampleCount = EMSAASampleCount::x8;

		ViewRenderDesc renderDesc
		{
			.ViewTransform = pCamera->GetViewTransform(),
			.SceneID = pEditor->GetActiveScene()->GetUUID(),
			.ViewID = GetUUID(),
			.RenderFeatures = RenderFeatures(),
			.RenderQualitySettings = renderQualitySettings,
			.MouseHoverCoordinates = IsClientAreaHovered() ? GetClientHoverCoordinates() : Vector2i(-1, -1),
			.RenderTarget = m_pRenderTarget,
			.Scene = pEditor->GetActiveScene()
		};

		const Vector2i& region = GetViewportSize();
		renderDesc.ViewTransform.Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));

		return renderDesc;
	}
}