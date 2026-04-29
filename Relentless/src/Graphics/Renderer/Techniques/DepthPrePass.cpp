#include "DepthPrePass.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

namespace Relentless
{
	DepthPrePass::DepthPrePass(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
	}

	void DepthPrePass::Render(CommandContext& commandContext, const RenderView& aRenderView, SceneTextures& sceneTextures) noexcept
	{
		RenderPassInfo info{};
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::Preserve;
		info.DepthStencilTarget.pTarget = sceneTextures.pDepthTarget;

		commandContext.InsertResourceBarrier(info.DepthStencilTarget.pTarget, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		commandContext.BeginRenderPass(info);

		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		Renderer::BindViewData(commandContext, aRenderView);

		//Opaque
		{
			PipelineStateInitializer psoDesc{};
			psoDesc.SetName("Depth Prepass - Opaque");
			psoDesc.SetDepthWrite(true);
			psoDesc.SetDepthEnabled(true);
			psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
			psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
			psoDesc.SetVertexShader("DepthPrePassShader", "vs_main");
			psoDesc.SetDepthOnlyTarget(ResourceFormat::D32_FLOAT, static_cast<uint32>(aRenderView.RenderQualitySettings.MSAASampleCount));

			commandContext.SetPipelineState(m_pDevice->GetOrCreatePipeline(psoDesc));
			Renderer::DrawScene(commandContext, aRenderView, Batch::Blending::Opaque);
		}

		commandContext.EndRenderPass();
	}
}