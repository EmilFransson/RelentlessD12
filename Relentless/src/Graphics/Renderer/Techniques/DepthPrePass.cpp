#include "DepthPrePass.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	DepthPrePass::DepthPrePass(GraphicsDevice* pDevice) noexcept
		: m_pGraphicsDevice{ pDevice }
	{
	}

	void DepthPrePass::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& sceneTextures) noexcept
	{
		RenderPassInfo info{};
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::Preserve;
		info.DepthStencilTarget.pTarget = sceneTextures.pDepthTarget;

		aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		aCommandContext.BeginRenderPass(info);

		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		aCommandContext.SetGraphicsRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());

		Renderer::BindViewData(aCommandContext, aRenderView);

		RenderOpaque(aCommandContext, aRenderView);
		RenderOpaqueTwoSided(aCommandContext, aRenderView);
		RenderAlphaMasked(aCommandContext, aRenderView);

		aCommandContext.EndRenderPass();
	}

	void DepthPrePass::RenderAlphaMasked(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		const uint32 numSamples = static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount);

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetName("Depth Prepass - Alpha Mask");
		psoDesc.SetAlphaToCoverageEnable(numSamples > 1u);
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main", { "ALPHA_MASK" });
		psoDesc.SetPixelShader("DepthPrePassShader", "ps_main", { "ALPHA_MASK" });
		psoDesc.SetDepthOnlyTarget(ResourceFormat::D32_FLOAT, numSamples);
		
		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreatePipeline(psoDesc));

		for (const Batch& batch : aRenderView.pRenderScene->GetBatches())
		{
			if (batch.BlendMode != Batch::Blending::AlphaMask)
				continue;

			if (batch.IsTwoSided)
				continue;
			
			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}

	void DepthPrePass::RenderOpaque(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Depth Prepass - Opaque");
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main");
		psoDesc.SetDepthOnlyTarget(ResourceFormat::D32_FLOAT, static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount));

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreatePipeline(psoDesc));
		
		for (const Batch& batch : aRenderView.pRenderScene->GetBatches())
		{
			if (batch.BlendMode != Batch::Blending::Opaque)
				continue;

			if (batch.IsTwoSided)
				continue;

			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}

	void DepthPrePass::RenderOpaqueTwoSided(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Depth Prepass - Opaque - TwoSided");
		psoDesc.SetCullMode(D3D12_CULL_MODE_NONE);
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main");
		psoDesc.SetDepthOnlyTarget(ResourceFormat::D32_FLOAT, static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount));

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreatePipeline(psoDesc));

		for (const Batch& batch : aRenderView.pRenderScene->GetBatches())
		{
			if (batch.BlendMode != Batch::Blending::Opaque)
				continue;

			if (!batch.IsTwoSided)
				continue;

			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}
}