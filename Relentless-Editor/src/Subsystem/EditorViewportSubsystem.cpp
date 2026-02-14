#include "EditorViewportSubsystem.h"

#include "Core/Editor.h"

#include "Module/ModuleManager.h"

#include "Subsystem/EditorRendererBridgeSubsystem.h"
#include "Subsystem/EditorSceneBridgeSubsystem.h"
#include "Subsystem/SelectionSubsystem.h"

namespace Relentless
{
	ViewportPanel* EditorViewportSubsystem::CreateViewportPanel() noexcept
	{
		UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
		ViewportPanel* pViewport = uiModule.AddPanel<ViewportPanel>(m_EditorViewports.size());
		pViewport->OnClickedOnViewport.Connect(this, &EditorViewportSubsystem::OnViewportClicked);
		pViewport->OnHotkeyPressed.Connect(this, &EditorViewportSubsystem::OnViewportHotkeyPressed);

		m_EditorViewports.push_back(pViewport);
		m_RenderViews.push_back(ViewportRenderView());

		return pViewport;
	}

	ViewportRenderView& EditorViewportSubsystem::GetRenderView(uint32 aRenderViewIndex) noexcept
	{
		RLS_ASSERT(m_RenderViews.size() > aRenderViewIndex, "[EditorViewportSubsystem::GetRenderView] Index Out Of Bounds Error.");
		return m_RenderViews[aRenderViewIndex];
	}

	std::vector<ViewportRenderView>& EditorViewportSubsystem::GetRenderViews() noexcept
	{
		return m_RenderViews;
	}

	bool EditorViewportSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		Editor* pEditor = static_cast<Editor*>(aSystemManager);
		m_pEditor = pEditor;

		m_OnUpdateCallbackID = pEditor->RegisterUpdateCallback(Callback<void(float)>::Bind(this, &EditorViewportSubsystem::OnUpdate));
		
		CreateViewportPanel();

		return true;
	}

	bool EditorViewportSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

	void EditorViewportSubsystem::OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept
	{
		for (size_t i = 0; i < m_EditorViewports.size(); ++i)
		{
			ViewportPanel* pViewportPanel = m_EditorViewports[i];

			const Vector2i& region = pViewportPanel->GetViewportSize();
			m_RenderViews[i].Viewport = FloatRect(0.0f, 0.0f, Math::Max(1.0f, (float)region.x), Math::Max(1.0f, (float)region.y));

			const ViewTransform& cameraViewTransform = pViewportPanel->GetCamera()->GetViewTransform();
			ViewportRenderView& renderView = m_RenderViews[i];

			renderView.Location = cameraViewTransform.Location;
			renderView.Viewport = cameraViewTransform.Viewport;
			renderView.IsPerspective = true;
			renderView.PerspectiveFrustum = cameraViewTransform.PerspectiveFrustum;
			renderView.OrthographicFrustum = cameraViewTransform.OrthographicFrustum;

			renderView.WorldToView = cameraViewTransform.WorldToView;
			renderView.WorldToClip = cameraViewTransform.WorldToClip;
			renderView.ViewToWorld = cameraViewTransform.ViewToWorld;
			renderView.ViewToClip = cameraViewTransform.ViewToClip;
			renderView.ClipToView = cameraViewTransform.ClipToView;

			renderView.FoV = cameraViewTransform.FoV;
			renderView.NearPlane = cameraViewTransform.NearPlane;
			renderView.FarPlane = cameraViewTransform.FarPlane;

			renderView.MouseHoverCoordinates = pViewportPanel->IsClientAreaHovered() ? pViewportPanel->GetClientHoverCoordinates() : Vector2i(-1, -1);

			renderView.MinLogLuminance = m_MinLogLuminance;
			renderView.MinEV100 = m_MinEV100;
			renderView.MaxEV100 = m_MaxEV100;
			renderView.ExposureCompensation = m_ExposureCompensation;
		}
	}

	void EditorViewportSubsystem::OnViewportClicked(MAYBE_UNUSED ViewportPanel* aPanel, MAYBE_UNUSED Vector2u aRelativeMouseCoords) noexcept
	{
		SelectionSubsystem* pSelection = m_pEditor->GetSubsystem<SelectionSubsystem>();
		EditorRendererBridgeSubsystem* pEditorRendererBridge = m_pEditor->GetSubsystem<EditorRendererBridgeSubsystem>();

		const entity hoveredEntity = pEditorRendererBridge->GetHoveredEntity();
		const bool lCtrlDown = Keyboard::IsKeyDown(RLS_Key::LCtrl);
		const bool lShiftDown = Keyboard::IsKeyDown(RLS_Key::LShift);
		const bool isHoveringEntity = hoveredEntity != NULL_ENTITY;

		if (!isHoveringEntity || (!lCtrlDown && !lShiftDown))
			pSelection->DeselectAllEntities();

		if (isHoveringEntity)
		{
			if (lCtrlDown && pSelection->IsEntitySelected(hoveredEntity))
				pSelection->DeselectEntity(hoveredEntity);
			else
				pSelection->SelectEntity(hoveredEntity);
		}
	}

	void EditorViewportSubsystem::OnViewportHotkeyPressed(MAYBE_UNUSED ViewportPanel* aPanel, RLS_Key aKey) noexcept
	{
		switch (aKey)
		{
		case RLS_Key::A:
			if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
				m_pEditor->GetSubsystem<EditorSceneBridgeSubsystem>()->SelectAllEntities();
			break;
		case RLS_Key::H:
			m_pEditor->GetSubsystem<EditorSceneBridgeSubsystem>()->SetVisibilityForSelectedEntities(Keyboard::IsKeyDown(RLS_Key::LCtrl));
			break;
		case RLS_Key::Delete:
			m_pEditor->GetSubsystem<EditorSceneBridgeSubsystem>()->DeleteSelectedEntities();
			break;
		default:
			break;
		}
	}
}