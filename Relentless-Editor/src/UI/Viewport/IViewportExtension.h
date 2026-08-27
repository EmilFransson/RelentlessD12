#pragma once

namespace Relentless
{
	class VerticalBox;
	struct ViewportInputEvent;
	struct ViewportToolbarSlots;
	class ViewportPanel;

	class IViewportExtension
	{
	public:
		virtual ~IViewportExtension() = default;

		virtual void ExtendToolbar(ViewportToolbarSlots&) {}
		virtual void ExtendSidePanel(Ref<VerticalBox>&) {}

		virtual bool HandleInput(const ViewportInputEvent&) { return false; }

		virtual void OnRegistered(ViewportPanel&) {}
		virtual void OnUnregister() {}
		virtual void OnUpdate(float) {}
		virtual void OnCanvasRenderEnd() {}

		NO_DISCARD virtual bool WantsExclusiveInput() const noexcept { return false; }
	};
}