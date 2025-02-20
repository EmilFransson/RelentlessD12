#include "ToyRenderer.h"

#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/Renderer/Renderer.h"

namespace Relentless
{
	ToyRenderer::ToyRenderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{pDevice}
	{
		Initialize();
	}

	void ToyRenderer::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept
	{
		const uint32 width = sceneTextures.pColorTarget->GetWidth();
		const uint32 height = sceneTextures.pColorTarget->GetHeight();
		const ResourceFormat colorFormat = sceneTextures.pColorTarget->GetFormat();

		Ref<TextureEx> pTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, colorFormat, 1u, TextureFlag::UnorderedAccess), "UAV Target");

		commandContext.SetPipelineState(m_pToyPSO);
		commandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());

		struct 
		{
			uint32 TargetIndex;
			uint32 w;
			uint32 h;
			uint32 pad0;
		} params;

		params.TargetIndex = pTarget->GetUAVIndex();
		params.w = pTarget->GetWidth();
		params.h = pTarget->GetHeight();

		commandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(commandContext, renderView);

		commandContext.Dispatch(ComputeUtils::GetNumThreadGroups(pTarget->GetWidth(), 16, pTarget->GetHeight(), 16));

		sceneTextures.pColorTarget = pTarget;
	}

	void ToyRenderer::Initialize() noexcept
	{
		m_pToyPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "ToyComputeShader");
	}

}
