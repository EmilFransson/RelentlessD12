#include "SkyBoxRenderer.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/Renderer/RenderTypes.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"

#include "Subsystem/CoreTypes/SkyBoxRenderSubsystem.h"

namespace Relentless
{
	SkyBoxRenderer::SkyBoxRenderer(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pGraphicsDevice{aGraphicsDevice}
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetName("Skybox");
		psoDesc.SetVertexShader("SkyboxShader", "vs_main");
		psoDesc.SetPixelShader("SkyboxShader", "ps_main");
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetCullMode(D3D12_CULL_MODE_NONE);
		psoDesc.SetRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, 1);

		m_pSkyBoxPSO = m_pGraphicsDevice->CreatePipeline(psoDesc);

		psoDesc.SetPixelShader("SkyboxShader", "ps_main", {"BLEND_ENVIRONMENTS"});
		m_pSkyBoxBlendPSO = m_pGraphicsDevice->CreatePipeline(psoDesc);
	}

	void SkyBoxRenderer::Render(const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		SkyBoxRenderSubsystem* pSkyBoxRenderSubsystem = aRenderView.pRenderer->GetSubsystem<SkyBoxRenderSubsystem>();
		CommandContext& commandContext = *m_pGraphicsDevice->AllocateCommandContext();

		RenderPassInfo info{};
		info.RenderTargets[0].pTarget = aSceneTextures.pColorTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Clear;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = aSceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ReadOnlyDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;

		commandContext.InsertResourceBarrier(aSceneTextures.pEnvironmentTarget, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		commandContext.InsertResourceBarrier(aSceneTextures.pColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);

		commandContext.BeginRenderPass(info);

		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.SetGraphicsRootSignature(m_pGraphicsDevice->GetGlobalRootSignature());
		commandContext.SetPipelineState(pSkyBoxRenderSubsystem->ShouldBlendEnvironments() ? m_pSkyBoxBlendPSO : m_pSkyBoxPSO);

		Renderer::BindViewData(commandContext, aRenderView);
		commandContext.Draw(0u, 36u, 0u, 1u);

		commandContext.EndRenderPass();

		commandContext.InsertResourceBarrier(aSceneTextures.pEnvironmentTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		commandContext.Execute();
	}
}