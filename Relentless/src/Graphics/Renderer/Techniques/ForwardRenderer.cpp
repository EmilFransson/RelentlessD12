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
	}

	void ForwardRenderer::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures) noexcept
	{
		if (!m_pForwardPSO || m_pForwardPSO->GetSampleCount() != sceneTextures.pColorTarget->GetSampleCount())
			Initialize(sceneTextures.pColorTarget->GetSampleCount());

		const uint32 width = sceneTextures.pColorTarget->GetWidth();
		const uint32 height = sceneTextures.pColorTarget->GetHeight();
		const ResourceFormat colorFormat = sceneTextures.pColorTarget->GetFormat();

		TextureDesc colorTargetDesc = TextureDesc::Create2D(
			width,
			height,
			colorFormat,
			1u, 
			TextureFlag::RenderTarget, 
			ClearBinding(Colors::Magenta), 
			sceneTextures.pColorTarget->GetSampleCount());

		TextureDesc depthTargetDesc = TextureDesc::Create2D(
			width,
			height,
			sceneTextures.pDepthTarget->GetFormat(),
			1u,
			TextureFlag::DepthStencil,
			ClearBinding(1.0f, 1u),
			sceneTextures.pDepthTarget->GetSampleCount());

		Ref<TextureEx> pColorTarget = m_pDevice->CreateTexture(colorTargetDesc, "Color Target");
		Ref<TextureEx> pDepthTarget = m_pDevice->CreateTexture(depthTargetDesc, "Depth Target");
		
		const bool resolveMSAA = pColorTarget->GetSampleCount() > 1;
		
		Ref<TextureEx> pResolveTarget = nullptr;
		if (resolveMSAA)
		{
			pResolveTarget = m_pDevice->CreateTexture(TextureDesc::Create2D(width, height, colorFormat), "Resolve target");
		}

		RenderPassInfo info;
		info.RenderTargets[0].pTarget = pColorTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Clear;
		info.RenderTargets[0].EndAccessFlags = resolveMSAA ? RenderTargetAccessFlags::Resolve : RenderTargetAccessFlags::Preserve;
		info.RenderTargets[0].pResolveTarget = pResolveTarget;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;

		commandContext.BeginRenderPass(info);
		
		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandContext.SetPipelineState(m_pForwardPSO);
		commandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		Renderer::BindViewData(commandContext, renderView);

		commandContext.Draw(0, 3, 0, 1);

		commandContext.EndRenderPass();

		if (pResolveTarget)
			sceneTextures.pColorTarget = pResolveTarget;
		else
			sceneTextures.pColorTarget = pColorTarget;
	}

	void ForwardRenderer::Initialize(uint32 samples) noexcept
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetAlphaToCoverageEnable(false);
		psoDesc.SetName("Forward - Opaque");
		psoDesc.SetVertexShader("NewVertexShader");
		psoDesc.SetPixelShader("NewPixelShader");
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
														//TODO: Change to HDR!!
		psoDesc.SetRenderTargetFormats(ResourceFormat::RGB10A2_UNORM, ResourceFormat::D32_FLOAT, samples);

		m_pForwardPSO = m_pDevice->CreatePipeline(psoDesc);
	}

}