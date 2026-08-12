#include <Relentless.h>

#include "Core/Editor.h"

#include "Panels/ViewportPanel.h"
#include "PickingViewportExtension.h"

#include "Subsystem/EditorRendererBridgeSubsystem.h"
#include "Subsystem/SelectionSubsystem.h"

#include "UI/Viewport/ViewportInputEvent.h"

namespace Relentless
{
	void PickingViewportExtension::OnRegistered(const ViewportPanel& aViewportPanel)
	{
		m_pViewportPanel = &aViewportPanel;
	}

	bool PickingViewportExtension::HandleInput(const ViewportInputEvent& aInputEvent)
	{
		switch (aInputEvent.Type)
		{
		case EViewportInputType::MouseButtonPressed:
		{
			if (aInputEvent.Button == RLS_Button::Left)
				m_DraggedSinceLeftPress = false;

			return false;
		}
		case EViewportInputType::MouseDragBegin:
		{
			m_DraggedSinceLeftPress = true;
			return false;
		}
		case EViewportInputType::MouseButtonReleased:
		{
			if (aInputEvent.Button != RLS_Button::Left)
				return false;

			if (m_DraggedSinceLeftPress)
			{
				m_DraggedSinceLeftPress = false;
				return false;
			}

			if (!aInputEvent.IsInsideClientArea())
				return false;

			PerformPick(aInputEvent);
			return true;
		}
		case EViewportInputType::FocusLost:
		{
			m_DraggedSinceLeftPress = false;
			return false;
		}
		default:
			return false;
		}
	}

	void PickingViewportExtension::PerformPick(const ViewportInputEvent& aInputEvent)
	{
		Editor* pEditor = Editor::Get();
		SelectionSubsystem* pSelection = pEditor->GetSubsystem<SelectionSubsystem>();
		EditorRendererBridgeSubsystem* pBridge = pEditor->GetSubsystem<EditorRendererBridgeSubsystem>();

		const entity hoveredEntity = pBridge->GetHoveredEntity();

		const bool ctrlDown = EnumHasAnyFlags(aInputEvent.KeyboardModifiers, KeyboardModifierMask::Ctrl);
		const bool shiftDown = EnumHasAnyFlags(aInputEvent.KeyboardModifiers, KeyboardModifierMask::Shift);
		const bool isHoveringEntity = hoveredEntity != NULL_ENTITY;

		if (!isHoveringEntity || (!ctrlDown && !shiftDown))
			pSelection->DeselectAllEntities();

		if (!isHoveringEntity)
			return;

		if (ctrlDown && pSelection->IsEntitySelected(hoveredEntity))
			pSelection->DeselectEntity(hoveredEntity);
		else
			pSelection->SelectEntity(hoveredEntity);
	}
}