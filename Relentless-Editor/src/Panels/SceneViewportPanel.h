#pragma once
#include "ViewportPanel.h"

namespace Relentless
{
	class SceneViewportPanel : public ViewportPanel
	{
	public:
		SceneViewportPanel(const char* aTitle) noexcept;
		virtual ~SceneViewportPanel() noexcept override = default;

		NO_DISCARD virtual ViewRenderDesc BuildRenderDescriptor() const noexcept override;

		NO_DISCARD virtual Scene* GetViewportScene() const noexcept = 0;
	};
}