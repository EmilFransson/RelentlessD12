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
		
		m_pCanvas->OnDragOver(this, &EditorViewportPanel::OnCanvasDragOverInternal);
		m_pCanvas->OnDrop(this, &EditorViewportPanel::OnDropOnCanvasInternal);

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

		RenderFeatures renderFeatures;

		RenderQualitySettings renderQualitySettings;
		renderQualitySettings.MSAASampleCount = EMSAASampleCount::x8;

		ViewRenderDesc renderDesc
		{
			.ViewTransform = pCamera->GetViewTransform(),
			.SceneID = pEditor->GetActiveScene()->GetUUID(),
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

	String EditorViewportPanel::GetDisplayName() const noexcept
	{
		return "Scene Viewport";
	}

	String EditorViewportPanel::GetPersistKey() const noexcept
	{
		return "Scene Viewport";
	}

	Reply EditorViewportPanel::OnCanvasDragOverInternal(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		const std::vector<Reply> replies = OnCanvasDragOver(aWidgetGeometry, aDragDropOperation);
		
		if (std::ranges::any_of(replies, [](const Reply& reply) { return reply.IsHandled(); }))
			return Reply::Handled();

		return Reply::Unhandled();
	}

	Reply EditorViewportPanel::OnDropOnCanvasInternal(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		const std::vector<Reply> replies = OnCanvasDrop(aWidgetGeometry, aDragDropOperation);

		if (std::ranges::any_of(replies, [](const Reply& reply) { return reply.IsHandled(); }))
			return Reply::Handled();

		return Reply::Unhandled();
	}
}