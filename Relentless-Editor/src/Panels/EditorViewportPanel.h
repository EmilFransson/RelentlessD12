#pragma once
#include <Relentless.h>

#include "ViewportPanel.h"

namespace Relentless
{
	class EditorViewportPanel : public ViewportPanel
	{
	public:
		EditorViewportPanel() noexcept;
		virtual ~EditorViewportPanel() noexcept override;

		NO_DISCARD virtual ViewRenderDesc BuildRenderDescriptor() const noexcept override;
		
		NO_DISCARD virtual String GetDisplayName() const noexcept override;
		NO_DISCARD virtual String GetPersistKey() const noexcept override;

		Broadcaster<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> OnCanvasDragOver;
		Broadcaster<Reply(const WidgetGeometry&, const Ref<DragDropOperationBase>&)> OnCanvasDrop;
	private:
		NO_DISCARD Reply OnCanvasDragOverInternal(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
		NO_DISCARD Reply OnDropOnCanvasInternal(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
	};
}