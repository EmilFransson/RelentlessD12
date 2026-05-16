#include "BlitPass.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	BlitPass::BlitPass(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{ aGraphicsDevice }
	{
	}

	void BlitPass::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, Ref<Texture> aBlitTarget) noexcept
	{
		struct  
		{
			uint32 SourceIndex	= std::numeric_limits<uint32>::max();
			uint32 TargetIndex	= std::numeric_limits<uint32>::max();
			float Padding0		= 0.0f;
			float Padding1		= 0.0f;
		} params;

		params.SourceIndex = aSceneTextures.pLDRColorTarget->GetSRVIndex();
		params.TargetIndex = aBlitTarget->GetUAVIndex();

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "BlitShader", "cs_main"));
		aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		
		Renderer::BindViewData(aCommandContext, aRenderView);

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aBlitTarget->GetWidth(), 16u, aBlitTarget->GetHeight(), 16u));
	}

}