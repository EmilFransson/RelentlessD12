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

	void PostProcessing::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, Ref<Buffer> aAverageLuminanceBuffer) noexcept
	{
		aCommandContext.InsertResourceBarrier(aSceneTextures.pHDRColorTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		aCommandContext.InsertResourceBarrier(aAverageLuminanceBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "PostProcessShader", "cs_main"));
		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		struct
		{
			uint32 SourceIndex;
			uint32 TargetIndex;
			uint32 AverageLuminanceIndex;
			float Padding;
		} params;

		params.SourceIndex = aSceneTextures.pHDRColorTarget->GetSRVIndex();
		params.TargetIndex = aSceneTextures.pLDRColorTarget->GetUAVIndex();
		params.AverageLuminanceIndex = aAverageLuminanceBuffer->GetSRVIndex();

		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(aCommandContext, aRenderView);

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aSceneTextures.pLDRColorTarget->GetWidth(), 16, aSceneTextures.pLDRColorTarget->GetHeight(), 16));
	}
}
