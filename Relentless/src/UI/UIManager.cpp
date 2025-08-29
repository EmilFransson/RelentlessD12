#include "UIManager.h"

#include "ContextMenu.h"
#include "EventSystem/IEvent.h"
#include "Panel.h"

namespace Relentless
{
	UIManager& UIManager::Get() noexcept
	{
		static UIManager uiManager;
		return uiManager;
	}

	bool UIManager::BeginDragDropSource(RequestDragDropOpFunc&& requestFunc) noexcept
	{
		if (!ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoPreviewTooltip))
			return false;
		
		if (!m_pDragDropOperation)
			m_pDragDropOperation = requestFunc();

		m_pDragDropOperation->Update();

		ImGui::SetDragDropPayload(m_pDragDropOperation->GetTypeName(), nullptr, 0);
		ImGui::EndDragDropSource();

		return true;
	}

	bool UIManager::BeginDragDropTarget(uint64 uniqueID, OnDragEnterFunc&& onDragEnterFunc, OnDragLeaveFunc&& onDragLeaveFunc, OnDropFunc&& onDropFunc) noexcept
	{
		if (!IsDragDropOperationValid())
			return false;

		auto& state = m_DragDropHoverStates[uniqueID];

		if (!ImGui::BeginDragDropTarget())
		{
			if (state.Over)
			{
				state.Over = false;
				onDragLeaveFunc(m_pDragDropOperation);
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
					m_DropTargetIsValid = onDragEnterFunc(m_pDragDropOperation);
				}
			}
			else
			{
				if (m_DropTargetIsValid)
				{
					onDropFunc(m_pDragDropOperation);
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
				onDragLeaveFunc(m_pDragDropOperation);
			}
		}

		ImGui::EndDragDropTarget();
	
		return true;
	}

	bool UIManager::IsAnyContextMenuActive() const noexcept
	{
		return m_pActiveContextMenu != nullptr;
	}

	bool UIManager::IsDragDropActive() const noexcept
	{
		return ImGui::IsDragDropActive();
	}

	bool UIManager::IsDragDropOperationValid() const noexcept
	{
		return m_pDragDropOperation != nullptr;
	}

	bool UIManager::OnEvent(IEvent& event) noexcept
	{
		for (const auto& pPanel : m_PanelStack)
		{
			if (!pPanel->IsVisible())
				continue;

			if (pPanel->OnEvent(event))
				return true;
		}

		return false;
	}

	void UIManager::OnRender() noexcept
	{
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

	void UIManager::OnUpdate() noexcept
	{
		if (m_PanelStackDirty)
			FocusSortPanelStack();

		for (auto& panel : m_PanelStack)
			panel->Update();

		if (!IsDragDropActive() && IsDragDropOperationValid())
			m_pDragDropOperation.Reset();
	}

	void UIManager::SetActiveContextMenu(Ref<ContextMenu> pContextMenu) noexcept
	{
		m_pActiveContextMenu = pContextMenu;
		m_pActiveContextMenu->OnClosed.Connect([this]() 
			{ 
				m_ShouldDestroyContextMenu = true;
				RLS_CORE_INFO("HI");
			});

		m_pActiveContextMenu->OnClosed.Connect(this, &UIManager::OnContextMenuClosed);

		m_ShouldDestroyContextMenu = false;
	}

	// ---------------- PRIVATE FUNCTIONS ------------------

	void UIManager::FocusSortPanelStack() noexcept
	{
		std::stable_sort(m_PanelStack.begin(), m_PanelStack.end(), [](const UniquePtr<PanelBase>& pPanelA, const UniquePtr<PanelBase>& pPanelB)
			{
				return pPanelA->GetLastFrameFocused() > pPanelB->GetLastFrameFocused();
			});

		m_PanelStackDirty = false;
	}

	void UIManager::OnContextMenuClosed() noexcept
	{
		//RLS_CORE_INFO("HI");
	}

	void UIManager::OnPanelGainedFocus(PanelBase*) noexcept
	{
		m_PanelStackDirty = true;
	}

}
