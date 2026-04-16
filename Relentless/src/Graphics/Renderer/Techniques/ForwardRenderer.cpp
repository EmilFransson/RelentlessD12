#include "ForwardRenderer.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"

namespace Relentless
{
	ForwardRenderer::ForwardRenderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetName("Forward - Opaque");
		psoDesc.SetVertexShader("ForwardShader", "vs_main");
		psoDesc.SetPixelShader("ForwardShader", "ps_main", {"RED_OUTPUT"});
		psoDesc.SetDepthWrite(false);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_EQUAL);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, 1);

		m_pForwardSolidPSO = m_pDevice->CreatePipeline(psoDesc);

		psoDesc.SetFillMode(D3D12_FILL_MODE_WIREFRAME);
		m_pForwardWireframePSO = m_pDevice->CreatePipeline(psoDesc);
	}

	void ForwardRenderer::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, RenderModeEx renderMode) noexcept
	{
		commandContext.InsertResourceBarrier(sceneTextures.pColorTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);

		RenderPassInfo info;
		info.RenderTargets[0].pTarget = sceneTextures.pColorTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = sceneTextures.pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ReadOnlyDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;

		commandContext.BeginRenderPass(info);
		
		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		Renderer::BindViewData(commandContext, renderView);

		//Opaque
		{
			if (renderMode == RenderModeEx::Solid)
				commandContext.SetPipelineState(m_pForwardSolidPSO);
			else
				commandContext.SetPipelineState(m_pForwardWireframePSO);

			Renderer::DrawScene(commandContext, renderView, Batch::Blending::Opaque);
		}

		commandContext.EndRenderPass();
	}
}