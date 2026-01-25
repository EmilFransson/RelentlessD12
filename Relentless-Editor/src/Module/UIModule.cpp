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
		for (const auto& pPanel : m_PanelStack)
		{
			if (!pPanel->IsVisible())
				continue;

			if (pPanel->OnEvent(aEvent))
				return true;
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
		if (m_PanelStackDirty)
			FocusSortPanelStack();

		for (auto& panel : m_PanelStack)
			panel->Update();

		if (!IsDragDropActive() && IsDragDropOperationValid())
			m_pDragDropOperation.Reset();
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

	void UIModule::OnPanelGainedFocus(MAYBE_UNUSED PanelBase* aPanel) noexcept
	{
		m_PanelStackDirty = true;
	}
}
