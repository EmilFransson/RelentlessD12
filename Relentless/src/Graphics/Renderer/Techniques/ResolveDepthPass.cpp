#include "ResolveDepthPass.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/Renderer/Renderer.h"

namespace Relentless
{
	ResolveDepthPass::ResolveDepthPass(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pDevice{ aGraphicsDevice }
	{
	}

	void ResolveDepthPass::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		struct
		{
			uint32 SourceIndex;
			uint32 TargetIndex;
			uint32 Size[2];
			uint32 NumSamples;
			uint32 Padding[3];
		} params;

		params.SourceIndex = aSceneTextures.pDepthTarget->GetSRVIndex();
		params.TargetIndex = aSceneTextures.pDepthIntermediateResolveTarget->GetUAVIndex();
		params.Size[0] = aSceneTextures.pDepthIntermediateResolveTarget->GetWidth();
		params.Size[1] = aSceneTextures.pDepthIntermediateResolveTarget->GetHeight();
		params.NumSamples = static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount);

		aCommandContext.InsertResourceBarrier(aSceneTextures.pDepthTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pDepthIntermediateResolveTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "ResolveDepthShader", "cs_main"));
		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(params.Size[0], 8, params.Size[1], 8));
		
		//Copy from resolve target to actual depth target:
		aCommandContext.InsertResourceBarrier(aSceneTextures.pDepthIntermediateResolveTarget, D3D12_RESOURCE_STATE_COPY_SOURCE);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pDepthResolveTarget, D3D12_RESOURCE_STATE_COPY_DEST);
		aCommandContext.CopyResource(aSceneTextures.pDepthIntermediateResolveTarget, aSceneTextures.pDepthResolveTarget);

		aSceneTextures.pDepthTarget = aSceneTextures.pDepthResolveTarget;
	}

}