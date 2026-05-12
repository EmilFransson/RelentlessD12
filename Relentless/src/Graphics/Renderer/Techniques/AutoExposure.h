#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class AutoExposure
	{
	public:
		AutoExposure(GraphicsDevice* pDevice) noexcept;
		void Render(CommandContext& commandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, SceneBuffers& aSceneBuffers) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}