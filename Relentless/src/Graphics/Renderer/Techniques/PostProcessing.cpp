#include "PostProcessing.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

#include "Subsystem/CoreTypes/PostProcessRenderSubsystem.h"

namespace Relentless
{
	PostProcessing::PostProcessing(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pDevice{ aGraphicsDevice }
	{
	}

	void PostProcessing::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, Ref<Buffer> aAverageLuminanceBuffer) noexcept
	{
		aCommandContext.InsertResourceBarrier(aSceneTextures.pHDRColorTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Bloom))
			aCommandContext.InsertResourceBarrier(aSceneTextures.pBloomUpscaleTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		if (aRenderView.RenderFeatures.IsEnabled(ERenderFeature::AutoExposure))
			aCommandContext.InsertResourceBarrier(aAverageLuminanceBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "PostProcessShader", "cs_main"));
		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		PostProcessRenderSubsystem* pPostProcessRenderSubsystem = aRenderView.pRenderScene->GetSubsystem<PostProcessRenderSubsystem>();
		const PostProcessRenderProxy& renderProxy = pPostProcessRenderSubsystem->GetRenderProxy();

		struct
		{
			uint32 SourceIndex;
			uint32 TargetIndex;
			uint32 AverageLuminanceIndex;
			uint32 BloomIndex;

			float BloomIntensity;
			float BloomBlendFactor;
			uint32 BloomDirtMaskIndex;
			float BloomDirtMaskIntensity;

			Vector4 BloomDirtMaskTint;
		} params;

		params.SourceIndex = aSceneTextures.pHDRColorTarget->GetSRVIndex();
		params.TargetIndex = aSceneTextures.pLDRColorTarget->GetUAVIndex();
		params.AverageLuminanceIndex = aRenderView.RenderFeatures.IsEnabled(ERenderFeature::AutoExposure) ? aAverageLuminanceBuffer->GetSRVIndex() : GraphicsCommon::GetDefaultBuffer(DefaultBufferType::Ones)->GetSRVIndex();
		params.BloomIndex = aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Bloom) ? aSceneTextures.pBloomUpscaleTarget->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::Black2D)->GetSRVIndex();
		params.BloomIntensity = renderProxy.BloomProxySettings.Intensity;
		params.BloomBlendFactor = 0.3f;
		params.BloomDirtMaskIndex = aRenderView.RenderFeatures.IsEnabled(ERenderFeature::Bloom) && renderProxy.BloomProxySettings.DirtMask ? renderProxy.BloomProxySettings.DirtMask->GetSRVIndex() : GraphicsCommon::GetDefaultTexture(DefaultTextureType::Black2D)->GetSRVIndex();
		params.BloomDirtMaskIntensity = renderProxy.BloomProxySettings.DirtMaskIntensity;
		params.BloomDirtMaskTint = renderProxy.BloomProxySettings.DirtMaskTint;

		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(aCommandContext, aRenderView);

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aSceneTextures.pLDRColorTarget->GetWidth(), 16, aSceneTextures.pLDRColorTarget->GetHeight(), 16));
	}
}
