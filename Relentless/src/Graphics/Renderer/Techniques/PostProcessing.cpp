#include "PostProcessing.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Renderer/Renderer.h"

namespace Relentless
{
	PostProcessing::PostProcessing(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
		m_pPostProcessPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "PostProcessShader", "cs_main");
	}

	void PostProcessing::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, Ref<Texture> pOutlinesSolidTexture, Ref<Texture> pOutlinesBlurredTexture) noexcept
	{
		const uint32 width = sceneTextures.pColorTarget->GetWidth();
		const uint32 height = sceneTextures.pColorTarget->GetHeight();
		const ResourceFormat colorFormat = ResourceFormat::RGB10A2_UNORM;

		Ref<Texture> pTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, colorFormat, 1u, TextureFlag::UnorderedAccess), "Linear Color Target");

		commandContext.SetPipelineState(m_pPostProcessPSO);
		commandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		struct
		{
			uint32 SourceIndex;
			uint32 TargetIndex;

			uint32 OutlinesSolidIndex;
			uint32 OutlinesBlurredIndex;
		} params;

		params.SourceIndex = sceneTextures.pColorTarget->GetSRVIndex();
		params.TargetIndex = pTarget->GetUAVIndex();

		params.OutlinesSolidIndex = pOutlinesSolidTexture->GetSRVIndex();
		params.OutlinesBlurredIndex = pOutlinesBlurredTexture->GetSRVIndex();

		commandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(commandContext, renderView);

		commandContext.Dispatch(ComputeUtils::GetNumThreadGroups(pTarget->GetWidth(), 16, pTarget->GetHeight(), 16));

		sceneTextures.pColorTarget = pTarget;
	}
}
