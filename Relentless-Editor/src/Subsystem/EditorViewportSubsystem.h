#pragma once
#include <Relentless.h>
#include "Panels/ViewportPanel.h"

namespace Relentless
{
	class EditorViewportPanel;

	class EditorViewportSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD virtual bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		void ConditionallyDragEntities() noexcept;

		void OnCanvasDragEnter(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation, EditorViewportPanel* aPanel) noexcept;
		void OnCanvasDragLeave(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation, EditorViewportPanel* aPanel) noexcept;
		NO_DISCARD Reply OnCanvasDragOver(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnDropOnCanvas(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		void OnPanelClose(PanelBase* aPanel) noexcept;
		void OnPanelOpen(PanelBase* aPanel) noexcept;
		void OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept;
		void OnViewportClicked(MAYBE_UNUSED ViewportPanel* aPanel, MAYBE_UNUSED Vector2u aRelativeMouseCoords) noexcept;
		void OnViewportHotkeyPressed(MAYBE_UNUSED ViewportPanel* aPanel, RLS_Key aKey) noexcept;
	private:
		std::vector<ViewportPanel*> m_EditorViewports;
		std::vector<entity> m_DraggedEntities;

		std::unordered_map<EditorViewportPanel*, CallbackID> m_CanvasDragEnterCallbackIDs;
		std::unordered_map<EditorViewportPanel*, CallbackID> m_CanvasDragLeaveCallbackIDs;

		CallbackID m_OnUpdateCallbackID = 0u;

		Editor* m_pEditor = nullptr;
		EditorViewportPanel* m_pDragContextPanel = nullptr;
	};
}