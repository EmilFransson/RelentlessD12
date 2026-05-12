#pragma once
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
	private:
		NO_DISCARD Reply OnCanvasDragOver(MAYBE_UNUSED const WidgetGeometry& aWidgetGeometry, const Ref<DragDropOperationBase>& aDragDropOperation) noexcept;
	};
}