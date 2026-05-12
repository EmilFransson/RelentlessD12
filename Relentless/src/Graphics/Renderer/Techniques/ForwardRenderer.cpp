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
			psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, numSamples);

			aCommandContext.SetPipelineState(m_pDevice->GetOrCreatePipeline(psoDesc));
			Renderer::DrawScene(aCommandContext, aRenderView, Batch::Blending::Opaque);
		}

		//Alpha-mask:
		{
			PipelineStateInitializer psoDesc{};
			psoDesc.SetBlendMode(BlendMode::Replace);
			psoDesc.SetAlphaToCoverageEnable(numSamples > 1);
			psoDesc.SetName("Forward - Alpha Mask");
			psoDesc.SetVertexShader("ForwardShader", "vs_main");
			psoDesc.SetPixelShader("ForwardShader", "ps_main", { "ALPHA_MASK" });
			psoDesc.SetDepthWrite(true);
			psoDesc.SetDepthEnabled(true);
			psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_EQUAL);
			psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
			psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, numSamples);
			PipelineState* pAlphaMaskFrontFace = m_pDevice->GetOrCreatePipeline(psoDesc);

			psoDesc.SetName("Forward - Alpha Mask - TwoSided - Cull Front");
			psoDesc.SetCullMode(D3D12_CULL_MODE_FRONT);
			psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
			PipelineState* pAlphaMaskBack = m_pDevice->GetOrCreatePipeline(psoDesc);

			psoDesc.SetName("Forward - Alpha Mask - TwoSided - Cull Back");
			psoDesc.SetCullMode(D3D12_CULL_MODE_BACK);
			psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
			PipelineState* pAlphaMaskFront = m_pDevice->GetOrCreatePipeline(psoDesc);

			for (const Batch& batch : aRenderView.pRenderScene->GetBatches())
			{
				if (batch.BlendMode != Batch::Blending::AlphaMask)
					continue;

				if (batch.IsTwoSided)
				{
					aCommandContext.SetPipelineState(pAlphaMaskBack);
					Renderer::SubmitBatch(aCommandContext, batch);
					aCommandContext.SetPipelineState(pAlphaMaskFront);
					Renderer::SubmitBatch(aCommandContext, batch);
				}
				else
				{
					aCommandContext.SetPipelineState(pAlphaMaskFrontFace);
					Renderer::SubmitBatch(aCommandContext, batch);
				}
			}
		}

		//Transparent
		{
			PipelineStateInitializer psoDesc{};
			psoDesc.SetBlendMode(BlendMode::Alpha);
			psoDesc.SetAlphaToCoverageEnable(false);
			psoDesc.SetCullMode(D3D12_CULL_MODE_FRONT);
			psoDesc.SetName("Forward - Transparent - BackFace");
			psoDesc.SetVertexShader("ForwardShader", "vs_main");
			psoDesc.SetPixelShader("ForwardShader", "ps_main");
			psoDesc.SetDepthWrite(false);
			psoDesc.SetDepthEnabled(true);
			psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
			psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
			psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, numSamples);

			PipelineState* pTransparentBackFacePSO = m_pDevice->GetOrCreatePipeline(psoDesc);

			psoDesc.SetCullMode(D3D12_CULL_MODE_BACK);
			psoDesc.SetName("Forward - Transparent - FrontFace");

			PipelineState* pTransparentFrontFacePSO = m_pDevice->GetOrCreatePipeline(psoDesc);

			for (const Batch& batch : aRenderView.pRenderScene->GetBatches())
			{
				if (batch.BlendMode != Batch::Blending::AlphaBlend)
					continue;

				if (batch.IsTwoSided)
				{
					aCommandContext.SetPipelineState(pTransparentBackFacePSO);
					Renderer::SubmitBatch(aCommandContext, batch);
				}
				
				aCommandContext.SetPipelineState(pTransparentFrontFacePSO);
				Renderer::SubmitBatch(aCommandContext, batch);
			}
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