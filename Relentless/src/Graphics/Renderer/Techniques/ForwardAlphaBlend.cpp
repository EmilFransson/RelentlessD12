#include "ForwardAlphaBlend.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/ResourceViews.h"

namespace Relentless
{
	ForwardAlphaBlend::ForwardAlphaBlend(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{ aGraphicsDevice }
	{
	}

	void ForwardAlphaBlend::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		const uint32 numSamples = static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount);

		RenderPassInfo info{};

		RenderPassInfo::RenderTargetInfo& renderTarget = info.AddRenderTarget();
		renderTarget.pTarget = aSceneTextures.pHDRColorTarget;
		renderTarget.pResolveTarget = numSamples > 1 ? aSceneTextures.pColorResolveTarget : nullptr;
		renderTarget.BeginAccessFlags = RenderTargetAccessFlags::Preserve;
		renderTarget.EndAccessFlags = numSamples > 1 ? RenderTargetAccessFlags::Resolve : RenderTargetAccessFlags::Preserve;

		info.DepthStencilTarget.pTarget = aSceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ReadOnlyDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::Preserve;

		if (renderTarget.pResolveTarget)
			aCommandContext.InsertResourceBarrier(renderTarget.pResolveTarget, D3D12_RESOURCE_STATE_RESOLVE_DEST);

		aCommandContext.InsertResourceBarrier(renderTarget.pTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_READ);

		aCommandContext.BeginRenderPass(info);

		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		aCommandContext.SetGraphicsRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());

		Renderer::BindViewData(aCommandContext, aRenderView);

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Alpha);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetCullMode(D3D12_CULL_MODE_FRONT);
		psoDesc.SetVertexShader("ForwardShader", "vs_main", { "ALPHA_BLEND"});
		psoDesc.SetPixelShader("ForwardShader", "ps_main", { "ALPHA_BLEND" });
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, numSamples);
		
		psoDesc.SetName("Forward - AlphaBlend - BackFace");
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		PipelineState* pAlphaBlendBackFacePSO = m_pGraphicsDevice->GetOrCreatePipeline(psoDesc);

		psoDesc.SetName("Forward - Transparent - FrontFace");
		psoDesc.SetCullMode(D3D12_CULL_MODE_BACK);
		PipelineState* pAlphaBlendFrontFacePSO = m_pGraphicsDevice->GetOrCreatePipeline(psoDesc);

		for (const Batch& batch : aRenderView.pRenderScene->GetBatches())
		{
			if (batch.BlendMode != Batch::Blending::AlphaBlend)
				continue;

			if (batch.IsTwoSided)
			{
				aCommandContext.SetPipelineState(pAlphaBlendBackFacePSO);
				Renderer::SubmitBatch(aCommandContext, batch);
			}

			aCommandContext.SetPipelineState(pAlphaBlendFrontFacePSO);
			Renderer::SubmitBatch(aCommandContext, batch);
		}

		aCommandContext.EndRenderPass();

		if (numSamples > 1)
			aSceneTextures.pHDRColorTarget = aSceneTextures.pColorResolveTarget;
	}

}