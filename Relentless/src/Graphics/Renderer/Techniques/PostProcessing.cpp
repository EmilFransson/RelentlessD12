#include "PostProcessing.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/Renderer/Renderer.h"

namespace Relentless
{
	PostProcessing::PostProcessing(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
		m_pPostProcessPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "PostProcessShader", "cs_main");
	}

	void PostProcessing::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, Ref<Texture> pOutlinesSolidTexture, Ref<Texture> pOutlinesBlurredTexture, Ref<Buffer> pAverageLuminanceBuffer, Ref<Texture> aFinalTexture) noexcept
	{
		//if (!m_pOutputTarget || m_pOutputTarget->GetWidth() != sceneTextures.pColorTarget->GetWidth() || m_pOutputTarget->GetHeight() != sceneTextures.pColorTarget->GetHeight()
		//	|| m_pOutputTarget->GetFormat() != sceneTextures.pColorTarget->GetFormat())
		//{
		//	const uint32 width = sceneTextures.pColorTarget->GetWidth();
		//	const uint32 height = sceneTextures.pColorTarget->GetHeight();
		//	const ResourceFormat colorFormat = ResourceFormat::RGB10A2_UNORM;
		//
		//	m_pOutputTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, colorFormat, 1u, TextureFlag::UnorderedAccess), "Linear Color Target");
		//}

		commandContext.InsertResourceBarrier(sceneTextures.pColorTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.InsertResourceBarrier(aFinalTexture, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		
		commandContext.InsertResourceBarrier(pOutlinesSolidTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.InsertResourceBarrier(pOutlinesBlurredTexture, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		commandContext.InsertResourceBarrier(pAverageLuminanceBuffer, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		commandContext.SetPipelineState(m_pPostProcessPSO);
		commandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

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

		params.SourceIndex = sceneTextures.pColorTarget->GetSRVIndex();
		params.TargetIndex = aFinalTexture->GetUAVIndex();

		params.OutlinesSolidIndex = pOutlinesSolidTexture->GetSRVIndex();
		params.OutlinesBlurredIndex = pOutlinesBlurredTexture->GetSRVIndex();
		params.AverageLuminanceIndex = pAverageLuminanceBuffer->GetSRVIndex();

		commandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(commandContext, renderView);

		commandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aFinalTexture->GetWidth(), 16, aFinalTexture->GetHeight(), 16));

		//sceneTextures.pColorTarget = m_pOutputTarget;
	}
}
