#include "EditorViewportPanel.h"

#include "Core/Editor.h"

#include "Extensions/EditorGridExtension.h"
#include "Extensions/PickingViewportExtension.h"
#include "Extensions/TransformGizmoViewportExtension.h"

#include "Graphics/Renderer/Renderer.h"

#include "ImGui/ImGuiFonts.h"

#include "UI/Views/Details/LayoutBuilders/ContextMenuBuilder.h"
#include "UI/Views/Details/ViewportDetailsView.h"
#include "UI/Viewport/ViewModeRegistry.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/Canvas.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	EditorViewportPanel::EditorViewportPanel() noexcept
		:SceneViewportPanel("Scene Viewport")
	{
		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer) { aRenderer->CreateView(viewID); });

		Editor* pEditor = Editor::Get();
		m_pScene = pEditor->GetActiveScene();

		pEditor->OnSceneChanged.Connect(this, &EditorViewportPanel::OnActiveSceneChanged);
	}

	EditorViewportPanel::~EditorViewportPanel() noexcept
	{
		Renderer::Dispatch([viewID = GetUUID()](Renderer* aRenderer) { aRenderer->DestroyView(viewID); });
		Editor::Get()->OnSceneChanged.Detach(this);
	}

	String EditorViewportPanel::GetDisplayName() const noexcept
	{
		return "Scene Viewport";
	}

	String EditorViewportPanel::GetPersistKey() const noexcept
	{
		return "Scene Viewport";
	}

	Scene* EditorViewportPanel::GetViewportScene() const noexcept
	{
		return m_pScene;
	}

	ViewportSidePanelDesc EditorViewportPanel::CreateSidePanelDesc()
	{
		return ViewportSidePanelDesc{ .Width = 250.0f, .StartVisible = false };
	}

	ViewportToolbarDesc EditorViewportPanel::CreateToolbarDesc()
	{
		return ViewportToolbarDesc{ .Height = 25.0f, .Margin = 3.0f, .GroupSpacing = 10.0f, };
	}

	void EditorViewportPanel::ExtendSidePanel(Ref<VerticalBox>& aVerticalBox)
	{
		aVerticalBox->AddWidget(RLS_NEW ViewportDetailsView(this))
			->SetHorizontalSizePolicy(ESizePolicy::Stretch)
			->SetVerticalSizePolicy(ESizePolicy::Stretch);
	}

	void EditorViewportPanel::ExtendToolbar(ViewportToolbarSlots& aToolbarSlots)
	{
		const ViewModeInfo& viewModeInfo = GetViewModeInfo(GetClient().GetViewMode());

		m_pViewModeButton = aToolbarSlots.Right->AddWidget(RLS_NEW Button(std::format("{} {} {}", viewModeInfo.Icon, viewModeInfo.DisplayName, ICON_FA_CHEVRON_DOWN)));
		m_pViewModeButton->OnClicked(this, &EditorViewportPanel::OnViewModeButtonClicked);
		m_pViewModeButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		m_pViewModeButton->SetTooltipText("View mode settings for the current viewport.");
		m_pViewModeButton->SetTextColor(Colors::TextInactive);
		m_pViewModeButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::TextDefault); });
		m_pViewModeButton->OnMouseExit([](Button* aButton) { aButton->SetTextColor(Colors::TextInactive); });
	}

	void EditorViewportPanel::OnInitialized()
	{
		Canvas* pCanvas = GetCanvas();
		pCanvas->OnDragEnter(this, &EditorViewportPanel::OnCanvasDragEnterInternal);
		pCanvas->OnDragLeave(this, &EditorViewportPanel::OnCanvasDragLeaveInternal);
		pCanvas->OnDragOver(this, &EditorViewportPanel::OnCanvasDragOverInternal);
		pCanvas->OnDrop(this, &EditorViewportPanel::OnDropOnCanvasInternal);

		GetClient().OnViewModeChanged.Connect(this, &EditorViewportPanel::OnRenderViewModeChanged);
	}

	void EditorViewportPanel::OnRenderViewModeChanged(ERenderViewMode aViewMode)
	{
		const ViewModeInfo& viewModeInfo = GetViewModeInfo(aViewMode);
		m_pViewModeButton->SetText(std::format("{} {} {}", viewModeInfo.Icon, viewModeInfo.DisplayName, ICON_FA_CHEVRON_DOWN));

		ViewportClient& client = GetClient();

		if (aViewMode == ERenderViewMode::Lit)
			client.EnableAllRenderFeatures();
		else if (aViewMode == ERenderViewMode::Unlit)
		{
			client.DisableAllRenderFeatures();
			client.SetRenderFeature(ERenderFeature::EntityPicking, true);
			client.SetRenderFeature(ERenderFeature::Outlines, true);
			client.SetRenderFeature(ERenderFeature::Grid, true);
			client.SetRenderFeature(ERenderFeature::ToneMap, true);
		}
		else
		{
			client.DisableAllRenderFeatures();
			client.SetRenderFeature(ERenderFeature::EntityPicking, true);
			client.SetRenderFeature(ERenderFeature::Outlines, true);
			client.SetRenderFeature(ERenderFeature::Grid, true);
		}
	}

	void EditorViewportPanel::OnViewModeButtonClicked()
	{
		static constexpr Color radioButtonColor = Color(1.0f, 1.0f, 1.0f, 0.75f);

		ContextMenuBuilder builder;

		builder.AddSection("View Mode")
			.Font(UI::Fonts::Get("Small"))
			.SeparatorColor(Color(1.0f, 1.0f, 1.0f, 0.25f))
			.TextColor(Colors::TextInactive)
			.Thickness(0.5f);

		for (const ViewModeInfo& viewModeInfo : GetViewModeRegistry())
		{
			builder.AddRadioButton(viewModeInfo.DisplayName)
				.Font(UI::Fonts::Get("Small"))
				.Icon(viewModeInfo.Icon)
				.TextColor(radioButtonColor)
				.Tooltip(viewModeInfo.Description)
				.Value([this, mode = viewModeInfo.Mode]() { return GetClient().GetViewMode() == mode; })
				.OnValueChanged([this, mode = viewModeInfo.Mode](bool) { GetClient().SetViewMode(mode); });
		}

		ModuleManager::LoadModuleChecked<UIModule>().SetActiveContextMenu(builder.BuildContextMenu());
	}

	void EditorViewportPanel::RegisterExtensions()
	{
		RegisterExtension<TransformGizmoViewportExtension>();
		RegisterExtension<PickingViewportExtension>();
		RegisterExtension<EditorGridExtension>();
	}

	void EditorViewportPanel::OnActiveSceneChanged(Scene* aScene) noexcept
	{
		m_pScene = aScene;
	}

	void EditorViewportPanel::OnCanvasDragEnterInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		OnCanvasDragEnter(aWidgetGeometry, aDragDropOperation);
	}

	void EditorViewportPanel::OnCanvasDragLeaveInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		OnCanvasDragLeave(aWidgetGeometry, aDragDropOperation);
	}

	Reply EditorViewportPanel::OnCanvasDragOverInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		const std::vector<Reply> replies = OnCanvasDragOver(aWidgetGeometry, aDragDropOperation);
		
		if (std::ranges::any_of(replies, [](const Reply& reply) { return reply.IsHandled(); }))
			return Reply::Handled();

		return Reply::Unhandled();
	}

	Reply EditorViewportPanel::OnDropOnCanvasInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept
	{
		const std::vector<Reply> replies = OnCanvasDrop(aWidgetGeometry, aDragDropOperation);

		if (std::ranges::any_of(replies, [](const Reply& reply) { return reply.IsHandled(); }))
			return Reply::Handled();

		return Reply::Unhandled();
	}
}