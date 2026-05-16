#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class BlitPass
	{
	public:
		BlitPass(GraphicsDevice* aGraphicsDevice) noexcept;
		
		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, Ref<Texture> aBlitTarget) noexcept;
	private:
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}