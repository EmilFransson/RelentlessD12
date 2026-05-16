#include "SelectionOutlinesCompositePass.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	SelectionOutlinesCompositePass::SelectionOutlinesCompositePass(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{ aGraphicsDevice }
	{
	}

	void SelectionOutlinesCompositePass::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesSolidTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesBlurTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		
		struct
		{
			uint32 TargetIndex			= std::numeric_limits<uint32>::max();
			uint32 OutlinesSolidIndex	= std::numeric_limits<uint32>::max();
			uint32 OutlinesBlurredIndex = std::numeric_limits<uint32>::max();
			float Padding				= 0.0f;
		} params;

		params.TargetIndex = aSceneTextures.pLDRColorTarget->GetUAVIndex();
		params.OutlinesSolidIndex = aSceneTextures.pOutlinesSolidTarget->GetSRVIndex();
		params.OutlinesBlurredIndex = aSceneTextures.pOutlinesBlurTarget->GetSRVIndex();

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreateComputePipeline(m_pGraphicsDevice->GetGlobalRootSignature(), "SelectionOutlinesCompositeShader", "cs_main"));
		aCommandContext.SetComputeRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));

		Renderer::BindViewData(aCommandContext, aRenderView);

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aSceneTextures.pLDRColorTarget->GetWidth(), 16u, aSceneTextures.pLDRColorTarget->GetHeight(), 16u));
	}
}