#include "UIModule.h"

#include <Core/Editor.h>

#include <Panels/EntityDetailsPanel.h>
#include <Panels/OutlinerPanel.h>
#include <Panels/WidgetShowcasePanel.h>

#include <UI/DragDrop/DragDropOperation.h>

namespace Relentless
{
	uint32 UIModule::AcquireSlot(const String& aPersistKey) noexcept
	{
		std::vector<bool>& slots = m_Slots[aPersistKey];
		for (size_t i = 0u; i < slots.size(); ++i)
		{
			if (!slots[i])
			{
				slots[i] = true;
				return static_cast<uint32>(i);
			}
		}

		slots.push_back(true);
		return static_cast<uint32>(slots.size() - 1);
	}

	// ---------------- PUBLIC FUNCTIONS ------------------

	void UIModule::ClearActiveDragDropOperation() noexcept
	{
		if (m_pDragDropOperation)
			OnDragDropOperationEnd(m_pDragDropOperation);
		
		m_pDragDropOperation = nullptr;
	}

	Ref<DragDropOperationBase> UIModule::GetActiveDragDropOperation() const noexcept
	{
		RLS_ASSERT(m_pDragDropOperation, "[UIModule::GetActiveDragDropOperation]: Drag drop operation is invalid");
		return m_pDragDropOperation;
	}

	bool UIModule::HasActiveDragDrop() const noexcept
	{
		return m_pDragDropOperation != nullptr;
	}

	bool UIModule::IsAnyContextMenuActive() const noexcept
	{
		return m_pActiveContextMenu != nullptr;
	}

	bool UIModule::IsDragDropActive() const noexcept
	{
		return ImGui::IsDragDropActive();
	}

	bool UIModule::IsDragDropOperationValid() const noexcept
	{
		return m_pDragDropOperation != nullptr;
	}

	bool UIModule::OnEvent(IEvent& aEvent) noexcept
	{
		const ImGuiIO& io = ImGui::GetIO();

		switch (aEvent.GetEventType())
		{
		case EventType::LeftMouseButtonPressedEvent:
		case EventType::RightMouseButtonPressedEvent:
		case EventType::MiddleMouseButtonPressedEvent:
		{
			if (m_pHoveredPanel && m_pHoveredPanel->AcceptsMouseInput())
			{
				m_pCaptureTargetPanel = m_pHoveredPanel;
				m_pCaptureTargetPanel->OnEvent(aEvent);
				return true;
			}

			if (io.WantCaptureMouse)
				return true;

			return false;
		}
		case EventType::MouseMovedEvent:
		{
			if (m_pCaptureTargetPanel)
			{
				m_pCaptureTargetPanel->OnEvent(aEvent);
				return true;
			}

			if (m_pHoveredPanel)
			{
				m_pHoveredPanel->OnEvent(aEvent);
				return true;
			}

			if (io.WantCaptureMouse)
				return true;

			return false;
		}
		case EventType::RawMouseMoveEvent:
		{
			RawMouseMoveEvent& rawMoveEvent = static_cast<RawMouseMoveEvent&>(aEvent);
			const Vector2i& deltaCoords = rawMoveEvent.GetDeltaCoordinates();
			
			if (Mouse::IsButtonDown(RLS_Button::Left) || Mouse::IsButtonDown(RLS_Button::Right) || Mouse::IsButtonDown(RLS_Button::Wheel))
			{
				if (m_IsDragging)
				{
					//Promote:
					MouseDragEvent dragEvent(deltaCoords);
					dragEvent.LeftButtonDown = Mouse::IsButtonDown(RLS_Button::Left);
					dragEvent.RightButtonDown = Mouse::IsButtonDown(RLS_Button::Right);
					dragEvent.WheelDown = Mouse::IsButtonDown(RLS_Button::Wheel);

					if (m_pCaptureTargetPanel)
						m_pCaptureTargetPanel->OnEvent(dragEvent);
				}
				else
				{
					m_CurrentDragOffset += Math::Sqrt(deltaCoords.x * deltaCoords.x + deltaCoords.y * deltaCoords.y);
					if (m_CurrentDragOffset >= m_DragThreshold)
					{
						m_IsDragging = true;
						MouseBeginDragEvent beginDragEvent;

						if (m_pCaptureTargetPanel)
							m_pCaptureTargetPanel->OnEvent(beginDragEvent);
					}
				}
			}

			if (m_pCaptureTargetPanel)
			{
				m_pCaptureTargetPanel->OnEvent(aEvent);
				return true;
			}

			if (io.WantCaptureMouse)
				return true;

			return false;
		}
		case EventType::LeftMouseButtonReleasedEvent:
		case EventType::RightMouseButtonReleasedEvent:
		case EventType::MiddleMouseButtonReleasedEvent:
		{
			if (m_IsDragging && !Mouse::IsButtonDown(RLS_Button::Left) && !Mouse::IsButtonDown(RLS_Button::Right) && !Mouse::IsButtonDown(RLS_Button::Wheel))
			{
				//Promote:
				MouseEndDragEvent endDragEvent;

				m_IsDragging = false;
				m_CurrentDragOffset = 0u;

				if (m_pCaptureTargetPanel)
				{
					m_pCaptureTargetPanel->OnEvent(endDragEvent);
					m_pCaptureTargetPanel->OnEvent(aEvent);
					m_pCaptureTargetPanel = nullptr;
				}
			}

			if (m_pCaptureTargetPanel)
			{
				m_pCaptureTargetPanel->OnEvent(aEvent);
				return true;
			}

			if (io.WantCaptureMouse)
				return true;

			return false;
		}
		case EventType::MouseWheelScrolledEvent:
		{
			if (m_pHoveredPanel)
				return m_pHoveredPanel->OnEvent(aEvent);

			if (io.WantCaptureMouse)
				return true;

			return false;
		}
		case EventType::KeyPressedEvent:
		case EventType::KeyReleasedEvent:
		{
			if (m_pFocusedPanel)
				return m_pFocusedPanel->OnEvent(aEvent);

			if (io.WantTextInput || io.WantCaptureKeyboard)
				return true;

			return false;
		}
		default:
			break;
		}

		return false;
	}

	void UIModule::OnRender() noexcept
	{
		PROFILE_FUNC;

		for (auto& panel : m_PanelStack)
			panel->Render();

		if (IsAnyContextMenuActive())
		{
			if (m_ShouldDestroyContextMenu)
				m_pActiveContextMenu.Reset();
			else
				m_pActiveContextMenu->Render();
		}

		if (m_pDragDropOperation && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
			ClearActiveDragDropOperation();
	}

	void UIModule::OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept
	{
		PROFILE_FUNC;

		ResolveOpenRequests();
		ResolveCloseRequests();

		if (m_PanelStackDirty)
			FocusSortPanelStack();

		for (auto& panel : m_PanelStack)
			panel->Update();
	}

	void UIModule::RequestClose(PanelBase* aPanel) noexcept
	{
		m_PanelsToClose.push_back(aPanel);
	}

	void UIModule::CreateDefaultPanels() noexcept
	{
		OpenPanel<EntityDetailsPanel>();
		OpenPanel<OutlinerPanel>();
		OpenPanel<WidgetShowcasePanel>();
	}

	void UIModule::SetActiveContextMenu(Ref<ContextMenu> aContextMenu) noexcept
	{
		m_pActiveContextMenu = aContextMenu;
		m_pActiveContextMenu->OnClosed.Connect([this]()
			{
				m_ShouldDestroyContextMenu = true;
			});

		m_ShouldDestroyContextMenu = false;
	}
	
	void UIModule::SetActiveDragDropOperation(Ref<DragDropOperationBase> aDragDropOperation) noexcept
	{
		m_pDragDropOperation = aDragDropOperation;
		m_pDragDropOperation->CreatePreview();
		OnDragDropOperationBegin(m_pDragDropOperation);
	}

	// ---------------- PROTECTED FUNCTIONS -----------------

	void UIModule::OnLoad()
	{
		Editor* pEditor = Editor::Get();
		m_OnUpdateCallbackID = pEditor->RegisterUpdateCallback(Callback<void(float)>::Bind(this, &UIModule::OnUpdate));
		m_OnRenderCallbackID = pEditor->RegisterUIRenderCallback(Callback<void()>::Bind(this, &UIModule::OnRender));
		m_OnEventCallbackID = pEditor->RegisterEventCallback(Callback<bool(IEvent&)>::Bind(this, &UIModule::OnEvent));

		CreateDefaultPanels();
	}

	void UIModule::OnUnload()
	{
		Editor* pEditor = Editor::Get();
		pEditor->UnregisterUpdateCallback(m_OnUpdateCallbackID);
		pEditor->UnregisterUIRenderCallback(m_OnRenderCallbackID);
		pEditor->UnregisterEventCallback(m_OnEventCallbackID);
		m_OnUpdateCallbackID = INVALID_CALLBACK_ID;
		m_OnRenderCallbackID = INVALID_CALLBACK_ID;
		m_OnEventCallbackID = INVALID_CALLBACK_ID;

		m_PanelStack.clear();
	}

	bool UIModule::SupportsAutomaticShutdown() const
	{
		return false;
	}

	// ---------------- PRIVATE FUNCTIONS ------------------

	void UIModule::FocusSortPanelStack() noexcept
	{
		std::stable_sort(m_PanelStack.begin(), m_PanelStack.end(), [](const UniquePtr<PanelBase>& aPanelA, const UniquePtr<PanelBase>& aPanelB)
			{
				return aPanelA->GetLastFrameFocused() > aPanelB->GetLastFrameFocused();
			});

		m_PanelStackDirty = false;
	}

	void UIModule::OnMouseEnterPanel(PanelBase* aPanel) noexcept
	{
		m_pHoveredPanel = aPanel;
	}

	void UIModule::OnMouseExitPanel(PanelBase* aPanel) noexcept
	{
		if (m_pHoveredPanel == aPanel)
			m_pHoveredPanel = nullptr;
	}

	void UIModule::OnPanelGainedFocus(PanelBase* aPanel) noexcept
	{
		m_pFocusedPanel = aPanel;
		m_PanelStackDirty = true;
	}

	void UIModule::OnPanelLostFocus(PanelBase* aPanel) noexcept
	{
		if (m_pFocusedPanel == aPanel)
			m_pFocusedPanel = nullptr;
	}

	void UIModule::ResolveCloseRequests() noexcept
	{
		if (m_PanelsToClose.empty())
			return;

		for (PanelBase* pPanel : m_PanelsToClose)
		{
			pPanel->OnClose(pPanel);
			OnPanelClose(pPanel);

			if (m_pHoveredPanel == pPanel)
				m_pHoveredPanel = nullptr;
			if (m_pFocusedPanel == pPanel)
				m_pFocusedPanel = nullptr;

			m_PanelStack.erase(std::remove_if(m_PanelStack.begin(), m_PanelStack.end(), [pPanel](const UniquePtr<PanelBase>& aPanel) { return aPanel.get() == pPanel; }), m_PanelStack.end());
		}

		m_PanelsToClose.clear();
	}

	void UIModule::ResolveOpenRequests() noexcept
	{
		for (const auto& pPanel : m_PendingPanelsToOpen)
			OnPanelOpen(pPanel.get());

		m_PanelStack.insert(m_PanelStack.end(),
			std::make_move_iterator(std::begin(m_PendingPanelsToOpen)),
			std::make_move_iterator(std::end(m_PendingPanelsToOpen)));

		m_PendingPanelsToOpen.clear();
	}
}
