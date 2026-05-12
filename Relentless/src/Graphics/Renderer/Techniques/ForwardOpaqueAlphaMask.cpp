#include "ForwardOpaqueAlphaMask.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"
#include "Graphics/RHI/ResourceViews.h"

namespace Relentless
{
	ForwardOpaqueAlphaMask::ForwardOpaqueAlphaMask(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{ aGraphicsDevice }
	{
	}

	void ForwardOpaqueAlphaMask::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		const uint32 numSamples = static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount);

		RenderPassInfo info{};

		RenderPassInfo::RenderTargetInfo& renderTarget = info.AddRenderTarget();
		renderTarget.pTarget = aSceneTextures.pColorTarget;
		renderTarget.pResolveTarget = nullptr;
		renderTarget.BeginAccessFlags = RenderTargetAccessFlags::Preserve;
		renderTarget.EndAccessFlags = RenderTargetAccessFlags::Preserve;

		info.DepthStencilTarget.pTarget = aSceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ReadOnlyDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::Preserve;

		aCommandContext.InsertResourceBarrier(renderTarget.pTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_READ);

		aCommandContext.BeginRenderPass(info);

		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		aCommandContext.SetGraphicsRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());

		Renderer::BindViewData(aCommandContext, aRenderView);

		RenderOpaque(aCommandContext, aRenderView);
		RenderOpaqueTwoSided(aCommandContext, aRenderView);
		RenderAlphaMasked(aCommandContext, aRenderView);

		aCommandContext.EndRenderPass();

		//Alpha Masked Two Sided requires depth write (less_equal) enabled and depth-DSV usage:
		aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::Preserve;

		aCommandContext.BeginRenderPass(info);
		RenderAlphaMaskedTwoSided(aCommandContext, aRenderView);
		aCommandContext.EndRenderPass();
	}

	void ForwardOpaqueAlphaMask::RenderAlphaMasked(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		const uint32 numSamples = static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount);

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(numSamples > 1);
		psoDesc.SetName("Forward - Alpha Mask");
		psoDesc.SetVertexShader("ForwardShader", "vs_main");
		psoDesc.SetPixelShader("ForwardShader", "ps_main", { "ALPHA_MASK" });
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_EQUAL);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, numSamples);

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

	void ForwardOpaqueAlphaMask::RenderAlphaMaskedTwoSided(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		const uint32 numSamples = static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount);

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetAlphaToCoverageEnable(numSamples > 1);
		psoDesc.SetVertexShader("ForwardShader", "vs_main");
		psoDesc.SetPixelShader("ForwardShader", "ps_main", { "ALPHA_MASK" });
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, numSamples);

		psoDesc.SetName("Forward - Alpha Mask - TwoSided - Cull Front");
		psoDesc.SetCullMode(D3D12_CULL_MODE_FRONT);
		PipelineState* pAlphaMaskBackFace = m_pGraphicsDevice->GetOrCreatePipeline(psoDesc);

		psoDesc.SetName("Forward - Alpha Mask - TwoSided - Cull Back");
		psoDesc.SetCullMode(D3D12_CULL_MODE_BACK);
		PipelineState* pAlphaMaskFrontFace = m_pGraphicsDevice->GetOrCreatePipeline(psoDesc);

		for (const Batch& batch : aRenderView.pRenderScene->GetBatches())
		{
			if (batch.BlendMode != Batch::Blending::AlphaMask)
				continue;

			if (!batch.IsTwoSided)
				continue;

			aCommandContext.SetPipelineState(pAlphaMaskBackFace);
			Renderer::SubmitBatch(aCommandContext, batch);
			aCommandContext.SetPipelineState(pAlphaMaskFrontFace);
			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}

	void ForwardOpaqueAlphaMask::RenderOpaque(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Forward - Opaque");
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetVertexShader("ForwardShader", "vs_main");
		psoDesc.SetPixelShader("ForwardShader", "ps_main");
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_EQUAL);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount));

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

	void ForwardOpaqueAlphaMask::RenderOpaqueTwoSided(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Forward - Opaque - TwoSided");
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetVertexShader("ForwardShader", "vs_main");
		psoDesc.SetPixelShader("ForwardShader", "ps_main");
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetCullMode(D3D12_CULL_MODE_NONE);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_EQUAL);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount));

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