#include "SkyBoxRenderer.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

#include "Subsystem/CoreTypes/SkyBoxRenderSubsystem.h"

namespace Relentless
{
	SkyBoxRenderer::SkyBoxRenderer(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{aGraphicsDevice}
	{
	}

	void SkyBoxRenderer::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		const bool isBlending = aRenderView.pRenderScene->GetSubsystem<SkyBoxRenderSubsystem>()->ShouldBlendEnvironments();

		RenderPassInfo info{};
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargets[0].pTarget = aSceneTextures.pColorTarget;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = aSceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ReadOnlyDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::Preserve;

		aCommandContext.InsertResourceBarrier(info.RenderTargets[0].pTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);
		aCommandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_READ);

		aCommandContext.BeginRenderPass(info);

		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		aCommandContext.SetGraphicsRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		
		static const std::vector<String> blendDefines = { "BLEND_ENVIRONMENTS" };
		static const std::vector<String> noDefines = {};

		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetName("Skybox");
		psoDesc.SetVertexShader("SkyboxShader", "vs_main");
		psoDesc.SetPixelShader("SkyboxShader", "ps_main", isBlending ? blendDefines : noDefines);
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetCullMode(D3D12_CULL_MODE_NONE);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount));

		aCommandContext.SetPipelineState(m_pGraphicsDevice->GetOrCreatePipeline(psoDesc));

		Renderer::BindViewData(aCommandContext, aRenderView);
		aCommandContext.Draw(0u, 36u, 0u, 1u);

		aCommandContext.EndRenderPass();
	}
}