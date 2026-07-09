#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/RHI.h"

namespace Relentless
{
	class ShadowMapping
	{
	public:
		ShadowMapping(GraphicsDevice* aGraphicsDevice) noexcept;

		void Render(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept;
	private:
		void RenderAlphaMasked(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept;
		void RenderOpaque(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept;
		void RenderOpaqueTwoSided(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept;
	private:
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}