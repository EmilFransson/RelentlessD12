#include "PostProcessing.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	PostProcessing::PostProcessing(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pDevice{ aGraphicsDevice }
	{
	}

	void PostProcessing::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, Ref<Buffer> aAverageLuminanceBuffer, Ref<Texture> aFinalTexture) noexcept
	{
		aCommandContext.InsertResourceBarrier(aSceneTextures.pColorTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesSolidTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesBlurTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		aCommandContext.InsertResourceBarrier(aAverageLuminanceBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "PostProcessShader", "cs_main"));
		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		struct
		{
			uint32 SourceIndex;
			uint32 TargetIndex;

			uint32 OutlinesSolidIndex;
			uint32 OutlinesBlurredIndex;

			uint32 AverageLuminanceIndex;
			float Padding0;
			float Padding1;
			float Padding2;
		} params;

		params.SourceIndex = aSceneTextures.pColorTarget->GetSRVIndex();
		params.TargetIndex = aFinalTexture->GetUAVIndex();

		params.OutlinesSolidIndex = aSceneTextures.pOutlinesSolidTarget->GetSRVIndex();
		params.OutlinesBlurredIndex = aSceneTextures.pOutlinesBlurTarget->GetSRVIndex();
		params.AverageLuminanceIndex = aAverageLuminanceBuffer->GetSRVIndex();

		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(aCommandContext, aRenderView);

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aFinalTexture->GetWidth(), 16, aFinalTexture->GetHeight(), 16));
	}
}
