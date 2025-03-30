#include "ForwardRenderer.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/TextureEx.h"

namespace Relentless
{
	ForwardRenderer::ForwardRenderer(GraphicsDevice* pDevice) noexcept
		: m_pDevice{ pDevice }
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetName("Forward - Opaque");
		psoDesc.SetVertexShader("EvolvingShader", "vs_main");
		psoDesc.SetPixelShader("EvolvingShader", "ps_main", {"RED_OUTPUT"});
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGBA32_FLOAT, ResourceFormat::D32_FLOAT, 1);

		m_pForwardSolidPSO = m_pDevice->CreatePipeline(psoDesc);

		psoDesc.SetFillMode(D3D12_FILL_MODE_WIREFRAME);
		m_pForwardWireframePSO = m_pDevice->CreatePipeline(psoDesc);
	}

	void ForwardRenderer::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, RenderModeEx renderMode) noexcept
	{
		const uint32 width = sceneTextures.pColorTarget->GetWidth();
		const uint32 height = sceneTextures.pColorTarget->GetHeight();
		const ResourceFormat colorFormat = sceneTextures.pColorTarget->GetFormat();

		const TextureDesc colorTargetDesc = TextureDesc::Create2D(
			width,
			height,
			colorFormat,
			1u, 
			TextureFlag::RenderTarget | TextureFlag::ShaderResource, 
			ClearBinding(Colors::LightSkyBlue),
			sceneTextures.pColorTarget->GetSampleCount());

		const TextureDesc depthTargetDesc = TextureDesc::Create2D(
			width,
			height,
			sceneTextures.pDepthTarget->GetFormat(),
			1u,
			TextureFlag::DepthStencil,
			ClearBinding(1.0f, 1u),
			sceneTextures.pDepthTarget->GetSampleCount());

		const Ref<TextureEx> pColorTarget = m_pDevice->CreateTexture(colorTargetDesc, "Color Target");
		const Ref<TextureEx> pDepthTarget = m_pDevice->CreateTexture(depthTargetDesc, "Depth Target");
		
		RenderPassInfo info;
		info.RenderTargets[0].pTarget = pColorTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Clear;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
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

		sceneTextures.pColorTarget = pColorTarget;
		sceneTextures.pDepthTarget = pDepthTarget;
	}
}