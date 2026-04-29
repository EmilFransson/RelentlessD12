#include "ForwardRenderer.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/ResourceViews.h"

namespace Relentless
{
	ForwardRenderer::ForwardRenderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
	}

	void ForwardRenderer::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		const uint32 numSamples = static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount);
		
		RenderPassInfo info{};

		RenderPassInfo::RenderTargetInfo& renderTarget = info.AddRenderTarget();
		renderTarget.BeginAccessFlags = RenderTargetAccessFlags::Preserve;
		renderTarget.pTarget = aSceneTextures.pColorTarget;
		renderTarget.pResolveTarget = numSamples > 1 ? aSceneTextures.pColorResolveTarget : nullptr;
		renderTarget.EndAccessFlags = RenderTargetAccessFlags::Preserve;

		info.DepthStencilTarget.pTarget = aSceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ReadOnlyDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::Preserve;

		aCommandContext.InsertResourceBarrier(renderTarget.pTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_READ);

		aCommandContext.BeginRenderPass(info);
		
		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		aCommandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		Renderer::BindViewData(aCommandContext, aRenderView);

		//Opaque
		{
			PipelineStateInitializer psoDesc{};
			psoDesc.SetBlendMode(BlendMode::Replace);
			psoDesc.SetAlphaToCoverageEnable(false);
			psoDesc.SetName("Forward - Opaque");
			psoDesc.SetVertexShader("ForwardShader", "vs_main");
			psoDesc.SetPixelShader("ForwardShader", "ps_main");
			psoDesc.SetDepthWrite(false);
			psoDesc.SetDepthEnabled(true);
			psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_EQUAL);
			psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
			psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount));

			aCommandContext.SetPipelineState(m_pDevice->GetOrCreatePipeline(psoDesc));
			Renderer::DrawScene(aCommandContext, aRenderView, Batch::Blending::Opaque);
		}

		aCommandContext.EndRenderPass();

		if (numSamples > 1)
		{
			aCommandContext.InsertResourceBarrier(info.RenderTargets[0].pTarget, D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
			aCommandContext.InsertResourceBarrier(info.RenderTargets[0].pResolveTarget, D3D12_RESOURCE_STATE_RESOLVE_DEST);
			aCommandContext.FlushResourceBarriers();
			aCommandContext.GetCommandList()->ResolveSubresourceRegion(info.RenderTargets[0].pResolveTarget->GetResource(), 0, 0, 0, info.RenderTargets[0].pTarget->GetResource(), 0, nullptr, D3D::ConvertFormat(info.RenderTargets[0].pResolveTarget->GetFormat()), D3D12_RESOLVE_MODE_AVERAGE);
		
			aSceneTextures.pColorTarget = info.RenderTargets[0].pResolveTarget;
		}
	}
}