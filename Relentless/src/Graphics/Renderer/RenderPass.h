#pragma once
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/RHI.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	struct RenderPassContext
	{
		const RenderView& RenderView;
		SceneTextures& SceneTextures;
	};

	class RenderPass
	{
	public:
		virtual void Render(CommandContext& aCommandContext, const RenderPassContext& aRenderPassContext) noexcept = 0;
		NO_DISCARD virtual bool ShouldRender(MAYBE_UNUSED const RenderPassContext& aRenderPassContext) const noexcept { return true; }
	};
}