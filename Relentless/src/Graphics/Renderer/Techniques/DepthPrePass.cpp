#include "DepthPrePass.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/TextureEx.h"

namespace Relentless
{
	DepthPrePass::DepthPrePass(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetDepthOnlyTarget(ResourceFormat::D32_FLOAT, 1);
		psoDesc.SetName("Depth Prepass - Opaque");
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
		psoDesc.SetVertexShader("DepthPrePassShader", "vs_main");

		m_pOpaquePSO = m_pDevice->CreatePipeline(psoDesc);
	}

	void DepthPrePass::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept
	{
		const uint32 width = sceneTextures.pDepthTarget->GetWidth();
		const uint32 height = sceneTextures.pDepthTarget->GetHeight();

		const TextureDesc depthTargetDesc = TextureDesc::Create2D(
			width,
			height,
			sceneTextures.pDepthTarget->GetFormat(),
			1u,
			TextureFlag::DepthStencil | TextureFlag::ShaderResource,
			ClearBinding(1.0f, 1u),
			sceneTextures.pDepthTarget->GetSampleCount());

		const Ref<TextureEx> pDepthTarget = m_pDevice->CreateTexture(depthTargetDesc, "Depth Target");

		RenderPassInfo info{};
		info.DepthStencilTarget.pTarget = pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;

		commandContext.BeginRenderPass(info);

		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		Renderer::BindViewData(commandContext, renderView);

		//Opaque
		{
			commandContext.SetPipelineState(m_pOpaquePSO);
			Renderer::DrawScene(commandContext, renderView, Batch::Blending::Opaque);
		}

		commandContext.EndRenderPass();

		sceneTextures.pDepthTarget = pDepthTarget;
	}

}