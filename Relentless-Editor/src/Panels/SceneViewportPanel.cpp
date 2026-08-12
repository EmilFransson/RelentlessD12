#include "SceneViewportPanel.h"

namespace Relentless
{
	SceneViewportPanel::SceneViewportPanel(const char* aTitle) noexcept
		: ViewportPanel(aTitle)
	{
	}

	ViewRenderDesc SceneViewportPanel::BuildRenderDescriptor() const noexcept
	{
		ViewRenderDesc desc = ViewportPanel::BuildRenderDescriptor();
		const Scene* pScene = GetViewportScene();
		desc.SceneID = pScene ? pScene->GetUUID() : NULL_UUID;

		return desc;
	}
}