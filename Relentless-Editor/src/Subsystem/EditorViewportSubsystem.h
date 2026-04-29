#pragma once
#include <Relentless.h>
#include "Panels/ViewportPanel.h"

namespace Relentless
{
	class EditorViewportSubsystem : public ISubsystem
	{
	public:
		NO_DISCARD virtual bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		void OnPanelClose(PanelBase* aPanel) noexcept;
		void OnPanelOpen(PanelBase* aPanel) noexcept;
		void OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept;
		void OnViewportClicked(MAYBE_UNUSED ViewportPanel* aPanel, MAYBE_UNUSED Vector2u aRelativeMouseCoords) noexcept;
		void OnViewportHotkeyPressed(MAYBE_UNUSED ViewportPanel* aPanel, RLS_Key aKey) noexcept;
	private:
		std::vector<ViewportPanel*> m_EditorViewports;

		CallbackID m_OnUpdateCallbackID = 0u;
		Editor* m_pEditor = nullptr;
	};
}