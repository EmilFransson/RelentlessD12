#include "Bloom.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	Bloom::Bloom(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{}

	void Bloom::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		uint32 numMips = aSceneTextures.pBloomDownsampleTarget->GetMipLevels();
		const uint32 width = aSceneTextures.pBloomDownsampleTarget->GetWidth();
		const uint32 height = aSceneTextures.pBloomDownsampleTarget->GetHeight();

		Ref<Texture> pSourceTexture = aSceneTextures.pHDRColorTarget;
		aCommandContext.InsertResourceBarrier(pSourceTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
		
		//Downsample:

		for (uint32 mip = 0; mip < numMips; ++mip)
		{
			const Vector2u targetDimensions(Math::Max(1u, width >> mip), Math::Max(1u, height >> mip));
			aCommandContext.InsertResourceBarrier(aSceneTextures.pBloomDownsampleTarget, D3D12_RESOURCE_STATE_UNKNOWN, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, mip);

			PipelineState* pPSO = mip == 0u 
				? m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "Bloom", "cs_downsample", { "KARIS_AVERAGE=1" }) 
				: m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "Bloom", "cs_downsample");
			aCommandContext.SetPipelineState(pPSO);

			struct
			{
				Vector2	TargetDimensionsInv;
				uint32	SourceMip;
				uint32	SourceIndex;
				uint32	TargetIndex;
				float	Paddding[3];
			} parameters;
			parameters.TargetDimensionsInv = Vector2(1.0f / targetDimensions.x, 1.0f / targetDimensions.y);
			parameters.SourceMip = mip == 0u ? 0u : mip - 1u;
			parameters.SourceIndex = pSourceTexture->GetSRVIndex();
			parameters.TargetIndex = aSceneTextures.pBloomDownsampleTarget->GetUAVIndex(mip);
			aCommandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(targetDimensions.x, 8, targetDimensions.y, 8));
			aCommandContext.InsertResourceBarrier(aSceneTextures.pBloomDownsampleTarget, D3D12_RESOURCE_STATE_UNKNOWN, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, mip);

			pSourceTexture = aSceneTextures.pBloomDownsampleTarget;
		}

		//Upscale:
		numMips = Math::Max(2u, numMips);
		Ref<Texture> pPreviousSource = aSceneTextures.pBloomDownsampleTarget;

		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		for (int32 mip = numMips - 2; mip >= 0; --mip)
		{
			const Vector2u targetDimensions(Math::Max(1u, width >> mip), Math::Max(1u, height >> mip));

			aCommandContext.InsertResourceBarrier(aSceneTextures.pBloomUpscaleTarget, D3D12_RESOURCE_STATE_UNKNOWN, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, mip);
			aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "Bloom", "cs_upscale"));

			struct
			{
				Vector2	TargetDimensionsInv;
				uint32	SourceCurrentMip;
				uint32	SourcePreviousMip;
				float	Radius;
				uint32	SourceIndex;
				uint32	PreviousSourceIndex;
				uint32	TargetIndex;
			} parameters;
			parameters.TargetDimensionsInv = Vector2(1.0f / targetDimensions.x, 1.0f / targetDimensions.y);
			parameters.SourceCurrentMip = mip;
			parameters.SourcePreviousMip = mip + 1u;
			parameters.Radius = 0.85f;
			parameters.SourceIndex = aSceneTextures.pBloomDownsampleTarget->GetSRVIndex();
			parameters.PreviousSourceIndex = pPreviousSource->GetSRVIndex();
			parameters.TargetIndex = aSceneTextures.pBloomUpscaleTarget->GetUAVIndex(mip);
			aCommandContext.BindRootCBV(BindingSlot::PerInstance, &parameters, sizeof(parameters));

			aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(targetDimensions.x, 8, targetDimensions.y, 8));

			aCommandContext.InsertResourceBarrier(aSceneTextures.pBloomUpscaleTarget, D3D12_RESOURCE_STATE_UNKNOWN, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE, mip);

			pPreviousSource = aSceneTextures.pBloomUpscaleTarget;
		}
	}
}