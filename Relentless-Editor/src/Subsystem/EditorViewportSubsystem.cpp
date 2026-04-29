#include "EditorViewportSubsystem.h"

#include "Core/Editor.h"

#include "Module/ModuleManager.h"
#include "Module/UIModule.h"

#include "Subsystem/EditorRendererBridgeSubsystem.h"
#include "Subsystem/EditorSceneBridgeSubsystem.h"
#include "Subsystem/SelectionSubsystem.h"

namespace Relentless
{
	bool EditorViewportSubsystem::OnLoad(ISystemManager* aSystemManager) noexcept
	{
		m_pEditor = static_cast<Editor*>(aSystemManager);
		m_OnUpdateCallbackID = m_pEditor->RegisterUpdateCallback(Callback<void(float)>::Bind(this, &EditorViewportSubsystem::OnUpdate));
		
		UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
		uiModule.OnPanelOpen.Connect(this, &EditorViewportSubsystem::OnPanelOpen);
		uiModule.OnPanelClose.Connect(this, &EditorViewportSubsystem::OnPanelClose);

		return true;
	}

	bool EditorViewportSubsystem::ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept
	{
		return dynamic_cast<Editor*>(aSystemManager) != nullptr;
	}

	void EditorViewportSubsystem::OnPanelClose(PanelBase* aPanel) noexcept
	{
		if (ViewportPanel* pViewportPanel = dynamic_cast<ViewportPanel*>(aPanel))
		{
			pViewportPanel->OnClickedOnViewport.Detach(this);
			pViewportPanel->OnHotkeyPressed.Detach(this);
			std::erase_if(m_EditorViewports, [pViewportPanel](ViewportPanel* aViewportPanel) { return aViewportPanel == pViewportPanel; });
		}
	}

	void EditorViewportSubsystem::OnPanelOpen(PanelBase* aPanel) noexcept
	{
		if (ViewportPanel* pViewportPanel = dynamic_cast<ViewportPanel*>(aPanel))
		{
			pViewportPanel->OnClickedOnViewport.Connect(this, &EditorViewportSubsystem::OnViewportClicked);
			pViewportPanel->OnHotkeyPressed.Connect(this, &EditorViewportSubsystem::OnViewportHotkeyPressed);
			m_EditorViewports.push_back(pViewportPanel);
		}
	}

	void EditorViewportSubsystem::OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept
	{
		std::vector<ViewRenderDesc> viewRenderDescs(m_EditorViewports.size());
		for (size_t i = 0; i < m_EditorViewports.size(); ++i)
			viewRenderDescs[i] = m_EditorViewports[i]->BuildRenderDescriptor();

		if (viewRenderDescs.empty())
			return;

		Renderer::Dispatch([renderDescs = std::move(viewRenderDescs)](Renderer* aRenderer) 
			{ 
				aRenderer->RenderViews(renderDescs); 
			});
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