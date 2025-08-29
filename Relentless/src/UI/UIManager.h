#pragma once

#include "DragDropOperation.h"

namespace Relentless
{
	class ContextMenu;
	class IEvent;
	class PanelBase;

	using RequestDragDropOpFunc = std::function<Ref<DragDropOperation>()>;
	using OnDragEnterFunc = std::function<bool(const Ref<DragDropOperation>&)>;
	using OnDragLeaveFunc = std::function<bool(const Ref<DragDropOperation>&)>;
	using OnDropFunc = std::function<bool(const Ref<DragDropOperation>&)>;

	class UIManager
	{
	public:
		static NO_DISCARD UIManager& Get() noexcept;

		template<typename PanelType>
		PanelType* AddPanel(UniquePtr<PanelType> pPanel) noexcept
		{
			pPanel->OnGainedFocus.Connect(this, &UIManager::OnPanelGainedFocus);
			m_PanelStack.push_back(std::move(pPanel));

			return static_cast<PanelType*>(m_PanelStack.back().get());
		}

		bool BeginDragDropSource(RequestDragDropOpFunc&& requestFunc) noexcept;
		bool BeginDragDropTarget(uint64 uniqueID, OnDragEnterFunc&& onDragEnterFunc, OnDragLeaveFunc&& onDragLeaveFunc, OnDropFunc&& onDropFunc) noexcept;

		NO_DISCARD bool IsAnyContextMenuActive() const noexcept;
		NO_DISCARD bool IsDragDropActive() const noexcept;
		NO_DISCARD bool IsDragDropOperationValid() const noexcept;

		NO_DISCARD bool OnEvent(IEvent& event) noexcept;
		
		void OnRender() noexcept;
		void OnUpdate() noexcept;

		void SetActiveContextMenu(Ref<ContextMenu> pContextMenu) noexcept;
	private:
		void FocusSortPanelStack() noexcept;
		void OnContextMenuClosed() noexcept;
		void OnPanelGainedFocus(PanelBase* pPanel) noexcept;
	private:
		std::vector<UniquePtr<PanelBase>> m_PanelStack;

		struct HoverState { bool Over = false; };
		std::unordered_map<uint64, HoverState> m_DragDropHoverStates;

		Ref<ContextMenu> m_pActiveContextMenu = nullptr;
		Ref<DragDropOperation> m_pDragDropOperation = nullptr;
		bool m_PanelStackDirty = false;
		bool m_ShouldDestroyContextMenu = false;
		bool m_DropTargetIsValid = false;
	};
}