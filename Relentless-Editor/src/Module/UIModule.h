#pragma once

#include <Event/EditorEvents.h>

#include <UI/Widgets/ContextMenu.h>
#include <UI/Widgets/DragDropOperation.h>
#include <UI/Widgets/Panel.h>

namespace Relentless
{
	class PanelBase;

	using RequestDragDropOpFunc = std::function<Ref<DragDropOperation>()>;
	using OnDragEnterFunc = std::function<bool(const Ref<DragDropOperation>&)>;
	using OnDragLeaveFunc = std::function<bool(const Ref<DragDropOperation>&)>;
	using OnDropFunc = std::function<bool(const Ref<DragDropOperation>&)>;

	class UIModule : public IModule
	{
	public:
		virtual ~UIModule() override = default;

		template<typename PanelType, typename... Args>
		PanelType* AddPanel(Args&&... someArgs) noexcept
		{
			UniquePtr<PanelType> pNewPanel = MakeUnique<PanelType>(std::forward<Args>(someArgs)...);
			pNewPanel->OnGainedFocus.Connect(this, &UIModule::OnPanelGainedFocus);
			pNewPanel->OnLostFocus.Connect(this, &UIModule::OnPanelLostFocus);
			pNewPanel->OnMouseEnter.Connect(this, &UIModule::OnMouseEnterPanel);
			pNewPanel->OnMouseExit.Connect(this, &UIModule::OnMouseExitPanel);

			m_PanelStack.push_back(std::move(pNewPanel));

			return static_cast<PanelType*>(m_PanelStack.back().get());
		}

		bool BeginDragDropSource(RequestDragDropOpFunc&& aRequestFunc) noexcept;
		bool BeginDragDropTarget(uint64 aUniqueID, OnDragEnterFunc&& aOnDragEnterFunc, OnDragLeaveFunc&& aOnDragLeaveFunc, OnDropFunc&& aOnDropFunc) noexcept;

		NO_DISCARD bool IsAnyContextMenuActive() const noexcept;
		NO_DISCARD bool IsDragDropActive() const noexcept;
		NO_DISCARD bool IsDragDropOperationValid() const noexcept;

		NO_DISCARD bool OnEvent(IEvent& aEvent) noexcept;

		void OnRender() noexcept;
		void OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept;

		void RequestClose(PanelBase* aPanel) noexcept;

		void SetActiveContextMenu(Ref<ContextMenu> aContextMenu) noexcept;
	protected:
		void OnLoad() override;
	private:
		void CreateDefaultPanels() noexcept;

		void FocusSortPanelStack() noexcept;

		void OnMouseEnterPanel(PanelBase* aPanel) noexcept;
		void OnMouseExitPanel(PanelBase* aPanel) noexcept;
		void OnPanelGainedFocus(PanelBase* aPanel) noexcept;
		void OnPanelLostFocus(PanelBase* aPanel) noexcept;

		void ResolveCloseRequests() noexcept;
	private:
		std::vector<UniquePtr<PanelBase>> m_PanelStack;
		std::vector<PanelBase*> m_PanelsToClose;

		PanelBase* m_pHoveredPanel			= nullptr;
		PanelBase* m_pFocusedPanel			= nullptr;
		PanelBase* m_pCaptureTargetPanel	= nullptr;

		struct HoverState { bool Over = false; };
		std::unordered_map<uint64, HoverState> m_DragDropHoverStates;

		Ref<ContextMenu> m_pActiveContextMenu = nullptr;
		Ref<DragDropOperation> m_pDragDropOperation = nullptr;
		bool m_PanelStackDirty = false;
		bool m_ShouldDestroyContextMenu = false;
		bool m_DropTargetIsValid = false;

		CallbackID m_OnUpdateCallbackID = 0;
		CallbackID m_OnRenderCallbackID = 0;
		CallbackID m_OnEventCallbackID = 0;

		inline static constexpr uint32 m_DragThreshold = 3u;
		uint32 m_CurrentDragOffset = 0u;
		bool m_IsDragging = false;
	};
}