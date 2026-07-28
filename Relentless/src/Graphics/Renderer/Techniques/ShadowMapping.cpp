#include "ShadowMapping.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

#include "Subsystem/CoreTypes/LightRenderSubsystem.h"

namespace Relentless
{
	ShadowMapping::ShadowMapping(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{ aGraphicsDevice }
	{
	}

	void ShadowMapping::Render(CommandContext& aCommandContext, const RenderView& aRenderView) noexcept
	{
		LightRenderSubsystem* pLightRenderSubsystem = aRenderView.pRenderScene->GetSubsystem<LightRenderSubsystem>();
		
		for (const auto& shadowView : pLightRenderSubsystem->GetShadowViews())
		{
			RenderPassInfo info = RenderPassInfo::DepthOnly(shadowView.DepthTexture, DepthTargetAccessFlags::ClearDepth, DepthTargetAccessFlags::Preserve);

			aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
			aCommandContext.BeginRenderPass(info);

			aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			aCommandContext.SetGraphicsRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());

			Renderer::BindViewData(aCommandContext, shadowView);

			RenderOpaque(aCommandContext, shadowView);
			RenderOpaqueTwoSided(aCommandContext, shadowView);
			RenderAlphaMasked(aCommandContext, shadowView);
			RenderAlphaMaskedTwoSided(aCommandContext, shadowView);

			aCommandContext.EndRenderPass();
			
			aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		}
	}

	void ShadowMapping::RenderAlphaMasked(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept
	{
		const bool isPerspective = aShadowView.IsPerspective;

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetName("Shadow Mapping - Alpha Mask");
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_GREATER);
		psoDesc.SetDepthBias(0, 0.0f, isPerspective ? -8.0f : 0);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main", { "ALPHA_MASK" });
		psoDesc.SetPixelShader("DepthPrePassShader", "ps_main", { "ALPHA_MASK" });
		psoDesc.SetDepthOnlyTarget(Renderer::ShadowFormat, 1u);

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreatePipeline(psoDesc));

		for (const Batch& batch : aShadowView.pRenderScene->GetBatches())
		{
			if (!batch.CastShadow)
				continue;

			if (batch.BlendMode != Batch::Blending::AlphaMask)
				continue;

			if (batch.IsTwoSided)
				continue;

			if (!EnumHasAnyFlags(batch.LightChannels, aShadowView.LightChannels))
				continue;

			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}

	void ShadowMapping::RenderAlphaMaskedTwoSided(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept
	{
		const bool isPerspective = aShadowView.IsPerspective;

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_GREATER);
		psoDesc.SetDepthBias(0, 0.0f, isPerspective ? -8.0f : 0);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main", { "ALPHA_MASK" });
		psoDesc.SetPixelShader("DepthPrePassShader", "ps_main", { "ALPHA_MASK" });
		psoDesc.SetDepthOnlyTarget(Renderer::ShadowFormat, 1u);

		psoDesc.SetName("Shadow Mapping - Alpha Mask - TwoSided - Cull Front");
		psoDesc.SetCullMode(D3D12_CULL_MODE_FRONT);
		PipelineState* pAlphaMaskBackFace = m_pGraphicsDevice->GetOrCreatePipeline(psoDesc);

		psoDesc.SetName("Shadow Mapping - Alpha Mask - TwoSided - Cull Back");
		psoDesc.SetCullMode(D3D12_CULL_MODE_BACK);
		PipelineState* pAlphaMaskFrontFace = m_pGraphicsDevice->GetOrCreatePipeline(psoDesc);

		for (const Batch& batch : aShadowView.pRenderScene->GetBatches())
		{
			if (!batch.CastShadow)
				continue;

			if (batch.BlendMode != Batch::Blending::AlphaMask)
				continue;

			if (!batch.IsTwoSided)
				continue;

			if (!EnumHasAnyFlags(batch.LightChannels, aShadowView.LightChannels))
				continue;

			aCommandContext.SetPipelineState(pAlphaMaskBackFace);
			Renderer::SubmitBatch(aCommandContext, batch);
			aCommandContext.SetPipelineState(pAlphaMaskFrontFace);
			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}

	void ShadowMapping::RenderOpaque(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept
	{
		const bool isPerspective = aShadowView.IsPerspective;

		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Shadow Mapping - Opaque");
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_GREATER);
		psoDesc.SetDepthBias(0, 0.0f, isPerspective ? -8.0f: 0);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main");
		psoDesc.SetDepthOnlyTarget(Renderer::ShadowFormat, 1u);

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreatePipeline(psoDesc));

		for (const Batch& batch : aShadowView.pRenderScene->GetBatches())
		{
			if (!batch.CastShadow)
				continue;

			if (batch.BlendMode != Batch::Blending::Opaque)
				continue;

			if (batch.IsTwoSided)
				continue;

			if (!EnumHasAnyFlags(batch.LightChannels, aShadowView.LightChannels))
				continue;

			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}

	void ShadowMapping::RenderOpaqueTwoSided(CommandContext& aCommandContext, const ShadowView& aShadowView) noexcept
	{
		const bool isPerspective = aShadowView.IsPerspective;

		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Shadow Mapping - Opaque - TwoSided");
		psoDesc.SetCullMode(D3D12_CULL_MODE_NONE);
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthBias(0, 0.0f, isPerspective ? -8.0f : 0);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_GREATER);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main");
		psoDesc.SetDepthOnlyTarget(Renderer::ShadowFormat, 1u);

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreatePipeline(psoDesc));

		for (const Batch& batch : aShadowView.pRenderScene->GetBatches())
		{
			if (!batch.CastShadow)
				continue;

			if (batch.BlendMode != Batch::Blending::Opaque)
				continue;

			if (!batch.IsTwoSided)
				continue;

			if (!EnumHasAnyFlags(batch.LightChannels, aShadowView.LightChannels))
				continue;

			Renderer::SubmitBatch(aCommandContext, batch);
		}
	}
}
