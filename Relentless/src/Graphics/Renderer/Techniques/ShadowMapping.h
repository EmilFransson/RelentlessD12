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
		void RenderAlphaMasked(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept;
		void RenderAlphaMaskedTwoSided(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept;
		void RenderOpaque(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept;
		void RenderOpaqueTwoSided(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept;
	private:
		GraphicsDevice* m_pGraphicsDevice = nullptr;
	};
}