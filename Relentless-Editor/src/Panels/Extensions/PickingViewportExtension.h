#pragma once
#include "UI/Viewport/IViewportExtension.h"

namespace Relentless
{
	class PickingViewportExtension final : public IViewportExtension
	{
	public:
		void OnRegistered(const ViewportPanel& aViewportPanel) override;
	protected:
		NO_DISCARD bool HandleInput(const ViewportInputEvent& aInputEvent) override;
	private:
		void PerformPick(const ViewportInputEvent& aInputEvent);
	private:
		const ViewportPanel* m_pViewportPanel = nullptr;
		bool m_DraggedSinceLeftPress = false;
	};
}