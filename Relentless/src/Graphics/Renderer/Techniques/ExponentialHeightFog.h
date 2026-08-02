#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class ExponentialHeightFog
	{
	public:
		ExponentialHeightFog(GraphicsDevice* aDevice) noexcept;
		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept;
	private:
		GraphicsDevice* m_pDevice = nullptr;
	};
}