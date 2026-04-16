#pragma once
#include <Relentless.h>
#include "Panels/ViewportPanel.h"

namespace Relentless
{
	class EditorViewportSubsystem : public ISubsystem
	{
	public:
		ViewportPanel* CreateViewportPanel() noexcept;

		NO_DISCARD ViewportRenderView& GetRenderView(uint32 aRenderViewIndex) noexcept;
		NO_DISCARD std::vector<ViewportRenderView>& GetRenderViews() noexcept;

		NO_DISCARD virtual bool OnLoad(ISystemManager* aSystemManager) noexcept override;
		
		static bool ShouldCreateSubsystem(ISystemManager* aSystemManager) noexcept;
	private:
		void OnUpdate(MAYBE_UNUSED float aDeltaTime) noexcept;
		void OnViewportClicked(MAYBE_UNUSED ViewportPanel* aPanel, MAYBE_UNUSED Vector2u aRelativeMouseCoords) noexcept;
		void OnViewportHotkeyPressed(MAYBE_UNUSED ViewportPanel* aPanel, RLS_Key aKey) noexcept;
	private:
		std::vector<ViewportRenderView> m_RenderViews;
		std::vector<ViewportPanel*> m_EditorViewports;

		float m_MinLogLuminance = -4.0f;
		float m_MinEV100 = -10.0f;
		float m_MaxEV100 = 20.0f;
		float m_ExposureCompensation = 2.0f;
		CallbackID m_OnUpdateCallbackID = 0u;
		Editor* m_pEditor = nullptr;
	};
}