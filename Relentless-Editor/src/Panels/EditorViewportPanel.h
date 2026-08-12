#pragma once
#include <Relentless.h>

#include "UI/DragDrop/DragDropOperation.h"

#include "SceneViewportPanel.h"

namespace Relentless
{
	class Button;
	struct WidgetGeometry;

	class EditorViewportPanel : public SceneViewportPanel
	{
	public:
		EditorViewportPanel() noexcept;
		virtual ~EditorViewportPanel() noexcept override;

		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;
		NO_DISCARD Scene* GetViewportScene() const noexcept override;

		Broadcaster<void(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> OnCanvasDragEnter;
		Broadcaster<void(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> OnCanvasDragLeave;
		Broadcaster<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> OnCanvasDragOver;
		Broadcaster<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> OnCanvasDrop;
	protected:
		NO_DISCARD ViewportSidePanelDesc CreateSidePanelDesc() override;
		NO_DISCARD ViewportToolbarDesc CreateToolbarDesc() override;

		void ExtendSidePanel(Ref<VerticalBox>& aVerticalBox) override;
		void ExtendToolbar(ViewportToolbarSlots& aToolbarSlots) override;

		void OnInitialized() override;
		void OnRenderViewModeChanged(ERenderViewMode aViewMode);
		void OnViewModeButtonClicked();

		virtual void RegisterExtensions() override;
	private:
		void OnActiveSceneChanged(Scene* aScene) noexcept;
		void OnCanvasDragEnterInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		void OnCanvasDragLeaveInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnCanvasDragOverInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnDropOnCanvasInternal(const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;

		Button* m_pViewModeButton = nullptr;
		Scene* m_pScene = nullptr;
	};
}