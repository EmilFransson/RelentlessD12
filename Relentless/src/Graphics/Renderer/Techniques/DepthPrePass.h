#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class DepthPrePass
	{
	public:
		DepthPrePass(GraphicsDevice* aGraphicsDevice) noexcept;
		void Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept;
	private:
		void RenderAlphaMasked(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept;
		void RenderOpaque(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept;
		void RenderOpaqueTwoSided(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept;
	private:
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}