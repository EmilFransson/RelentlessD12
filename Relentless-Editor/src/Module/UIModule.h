#pragma once

#include "Event/EditorEvents.h"

#include "Panels/Panel.h"

#include "UI/Widgets/ContextMenu.h"

namespace Relentless
{
	class DragDropOperationBase;
	class PanelBase;

	class UIModule : public IModule
	{
	public:
		virtual ~UIModule() override = default;

		NO_DISCARD uint32 AcquireSlot(const String& aPersistKey) noexcept;

		template<typename PanelType, typename... Args>
		PanelType* OpenPanel(Args&&... someArgs) noexcept
		{
			UniquePtr<PanelType> pNewPanel = MakeUnique<PanelType>(std::forward<Args>(someArgs)...);
			pNewPanel->OnGainedFocus.Connect(this, &UIModule::OnPanelGainedFocus);
			pNewPanel->OnLostFocus.Connect(this, &UIModule::OnPanelLostFocus);
			pNewPanel->OnMouseEnter.Connect(this, &UIModule::OnMouseEnterPanel);
			pNewPanel->OnMouseExit.Connect(this, &UIModule::OnMouseExitPanel);

			pNewPanel->InitializeIdentity(AcquireSlot(pNewPanel->GetPersistKey()));

			m_PendingPanelsToOpen.push_back(std::move(pNewPanel));

			return static_cast<PanelType*>(m_PendingPanelsToOpen.back().get());
		}

		void ClearActiveDragDropOperation() noexcept;

		void DestroyActiveContextMenu() noexcept;

		NO_DISCARD Ref<DragDropOperationBase> GetActiveDragDropOperation() const noexcept;

		NO_DISCARD bool HasActiveDragDrop() const noexcept;

		NO_DISCARD bool IsAnyContextMenuActive() const noexcept;
		NO_DISCARD bool IsDragDropActive() const noexcept;
		NO_DISCARD bool IsDragDropOperationValid() const noexcept;

		NO_DISCARD bool OnEvent(IEvent& aEvent) noexcept;

		void OnRender() noexcept;
		void OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept;

		void RequestClose(PanelBase* aPanel) noexcept;

		void SetActiveContextMenu(Ref<ContextMenu> aContextMenu) noexcept;
		void SetActiveDragDropOperation(Ref<DragDropOperationBase> aDragDropOperation) noexcept;

		Broadcaster<void(const Ref<DragDropOperationBase>& aDragDropOperation)> OnDragDropOperationBegin;
		Broadcaster<void(const Ref<DragDropOperationBase>& aDragDropOperation)> OnDragDropOperationEnd;

		Broadcaster<void(PanelBase*)> OnPanelOpen;
		Broadcaster<void(PanelBase*)> OnPanelClose;
	protected:
		void OnLoad() override;
		void OnUnload() override;

		NO_DISCARD bool SupportsAutomaticShutdown() const override;
	private:
		void CreateDefaultPanels() noexcept;

		void FocusSortPanelStack() noexcept;

		void OnMouseEnterPanel(PanelBase* aPanel) noexcept;
		void OnMouseExitPanel(PanelBase* aPanel) noexcept;
		void OnPanelGainedFocus(PanelBase* aPanel) noexcept;
		void OnPanelLostFocus(PanelBase* aPanel) noexcept;

		void ResolveCloseRequests() noexcept;
		void ResolveOpenRequests() noexcept;
	private:
		std::vector<UniquePtr<PanelBase>> m_PanelStack;
		std::vector<PanelBase*> m_PanelsToClose;
		std::vector<UniquePtr<PanelBase>> m_PendingPanelsToOpen;
		std::unordered_map<String, std::vector<bool>> m_Slots;

		PanelBase* m_pHoveredPanel			= nullptr;
		PanelBase* m_pFocusedPanel			= nullptr;
		PanelBase* m_pCaptureTargetPanel	= nullptr;

		struct HoverState { bool Over = false; };
		std::unordered_map<uint64, HoverState> m_DragDropHoverStates;

		Ref<ContextMenu> m_pActiveContextMenu;
		Ref<DragDropOperationBase> m_pDragDropOperation = nullptr;
		bool m_PanelStackDirty = false;
		bool m_ShouldDestroyContextMenu = false;

		CallbackID m_OnUpdateCallbackID = 0;
		CallbackID m_OnRenderCallbackID = 0;
		CallbackID m_OnEventCallbackID = 0;

		inline static constexpr uint32 m_DragThreshold = 3u;
		uint32 m_CurrentDragOffset = 0u;
		bool m_IsDragging = false;
	};
}