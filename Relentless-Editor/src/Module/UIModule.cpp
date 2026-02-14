#include "UIModule.h"

#include <Core/Editor.h>

#include <Panels/EntityDetailsPanel.h>
#include <Panels/OutlinerPanel.h>
#include <Panels/WidgetShowcasePanel.h>

namespace Relentless
{
	// ---------------- PUBLIC FUNCTIONS ------------------

	bool UIModule::BeginDragDropSource(RequestDragDropOpFunc&& aRequestFunc) noexcept
	{
		if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoPreviewTooltip))
			return false;

		if (!m_pDragDropOperation)
			m_pDragDropOperation = aRequestFunc();

		m_pDragDropOperation->Update();

		ImGui::SetDragDropPayload(m_pDragDropOperation->GetTypeName(), nullptr, 0);
		ImGui::EndDragDropSource();

		return true;
	}

	bool UIModule::BeginDragDropTarget(uint64 aUniqueID, OnDragEnterFunc&& aOnDragEnterFunc, OnDragLeaveFunc&& aOnDragLeaveFunc, OnDropFunc&& aOnDropFunc) noexcept
	{
		if (!IsDragDropOperationValid())
			return false;

		auto& state = m_DragDropHoverStates[aUniqueID];

		if (!ImGui::BeginDragDropTarget())
		{
			if (state.Over)
			{
				state.Over = false;
				aOnDragLeaveFunc(m_pDragDropOperation);
			}

			return false;
		}

		if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(m_pDragDropOperation->GetTypeName(), ImGuiDragDropFlags_AcceptBeforeDelivery))
		{
			if (!payload->IsDelivery())
			{
				if (!state.Over)
				{
					state.Over = true;
					m_DropTargetIsValid = aOnDragEnterFunc(m_pDragDropOperation);
				}
			}
			else
			{
				if (m_DropTargetIsValid)
				{
					aOnDropFunc(m_pDragDropOperation);
					m_pDragDropOperation.Reset();
					m_DragDropHoverStates.clear();
					m_DropTargetIsValid = false;
				}
			}
		}
		else
		{
			if (state.Over)
			{
				state.Over = false;
				aOnDragLeaveFunc(m_pDragDropOperation);
			}
		}

		ImGui::EndDragDropTarget();

		return true;
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
	}

	void UIModule::OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept
	{
		PROFILE_FUNC;

		ResolveCloseRequests();

		if (m_PanelStackDirty)
			FocusSortPanelStack();

		for (auto& panel : m_PanelStack)
			panel->Update();

		if (!IsDragDropActive() && IsDragDropOperationValid())
			m_pDragDropOperation.Reset();
	}

	void UIModule::RequestClose(PanelBase* aPanel) noexcept
	{
		m_PanelsToClose.push_back(aPanel);
	}

	void UIModule::CreateDefaultPanels() noexcept
	{
		AddPanel<EntityDetailsPanel>();
		AddPanel<OutlinerPanel>();
		AddPanel<WidgetShowcasePanel>();
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
	
	// ---------------- PROTECTED FUNCTIONS -----------------

	void UIModule::OnLoad()
	{
		Editor* pEditor = Editor::Get();
		m_OnUpdateCallbackID = pEditor->RegisterUpdateCallback(Callback<void(float)>::Bind(this, &UIModule::OnUpdate));
		m_OnRenderCallbackID = pEditor->RegisterUIRenderCallback(Callback<void()>::Bind(this, &UIModule::OnRender));
		m_OnEventCallbackID = pEditor->RegisterEventCallback(Callback<bool(IEvent&)>::Bind(this, &UIModule::OnEvent));

		CreateDefaultPanels();
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

			if (m_pHoveredPanel == pPanel)
				m_pHoveredPanel = nullptr;
			if (m_pFocusedPanel == pPanel)
				m_pFocusedPanel = nullptr;

			m_PanelStack.erase(std::remove_if(m_PanelStack.begin(), m_PanelStack.end(), [pPanel](const UniquePtr<PanelBase>& aPanel) { return aPanel.get() == pPanel; }), m_PanelStack.end());
		}

		m_PanelsToClose.clear();
	}

}
