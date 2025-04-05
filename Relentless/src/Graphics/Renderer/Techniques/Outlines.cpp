#include "Outlines.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"

#include "Scene/Scene.h"

namespace Relentless
{
	constexpr uint32 RADIUS = 3u;
	
	Outlines::Outlines(GraphicsDevice* pDevice) noexcept
		: m_pDevice{pDevice}
	{
		PipelineStateInitializer psoDesc{};
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetDepthWrite(true);
		psoDesc.SetVertexShader("EntityOutputShader", "vs_main");
		psoDesc.SetPixelShader("EntityOutputShader", "ps_main");
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::R32_FLOAT, ResourceFormat::D32_FLOAT, 1);

		psoDesc.SetName("Outlines - Solid");

		m_pSolidPSO = m_pDevice->CreatePipeline(psoDesc);
		m_pGaussianBlurPSO = m_pDevice->CreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "GaussianBlurSeparable", "cs_main");

		constexpr float sigma = static_cast<float>(RADIUS) / 2.0f;
		float sum = 0.0f;

		// Create unnormalized weights
		for (int i = -int(RADIUS); i <= int(RADIUS); ++i)
		{
			const float weight = std::exp(-(i * i) / (2.0f * sigma * sigma));
			m_CBData.Weights[i + RADIUS].value = weight;
			sum += weight;
		}

		// Normalize
		for (int i = 0; i < int(kWeightCount); ++i) 
		{
			m_CBData.Weights[i].value /= sum;
		}
	}

	void Outlines::Render(CommandContext& commandContext, const RenderView& renderView, SceneTextures& sceneTextures, Ref<TextureEx> pEntityIDTexture) noexcept
	{
		const uint32 width = sceneTextures.pColorTarget->GetWidth();
		const uint32 height = sceneTextures.pColorTarget->GetHeight();
		const ResourceFormat colorFormat = ResourceFormat::R32_FLOAT;

		const TextureDesc colorTargetDesc = TextureDesc::Create2D(
			width,
			height,
			colorFormat,
			1u,
			TextureFlag::RenderTarget | TextureFlag::ShaderResource,
			ClearBinding(Colors::Black),
			sceneTextures.pColorTarget->GetSampleCount());
		
		const TextureDesc depthTargetDesc = TextureDesc::Create2D(
			width,
			height,
			sceneTextures.pDepthTarget->GetFormat(),
			1u,
			TextureFlag::DepthStencil,
			ClearBinding(1.0f, 1u),
			sceneTextures.pDepthTarget->GetSampleCount());
		
		m_pSolidOutput = m_pDevice->CreateTexture(colorTargetDesc, "Color Target");
		const Ref<TextureEx> pDepthTarget = m_pDevice->CreateTexture(depthTargetDesc, "Depth Target");
		
		RenderPassInfo info;
		info.RenderTargets[0].pTarget = m_pSolidOutput;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Clear;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;
		
		info.DepthStencilTarget.pTarget = pDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;
		
		commandContext.BeginRenderPass(info);
		
		commandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		commandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());
		commandContext.SetPipelineState(m_pSolidPSO);
		
		Renderer::BindViewData(commandContext, renderView);
		
		EntityManager& entityManager = renderView.pScene->GetEntityManager();
		
		auto batches = renderView.pRenderer->GetBatches();
		for (const Batch& batch : batches)
		{
			if (entityManager.Has<SelectedInEditorComponent>(batch.EntityID))
			{
				struct
				{
					uint32 InstanceIndex;
					uint32 EntityID;
				} params;
		
				params.InstanceIndex = batch.InstanceID;
				params.EntityID = entityManager.GetIdentity(batch.EntityID) + 1;
		
				commandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&params, sizeof(params));
		
				const uint32 numIndices = batch.pMesh->GetIndexBuffer()->GetNrOfElements();
				commandContext.Draw(0u, numIndices, 0u, 1u);
			}
		}
		
		commandContext.EndRenderPass();
	
		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

		const TextureDesc blurredColorTargetDesc = TextureDesc::Create2D(
			width,
			height,
			colorFormat,
			1u,
			TextureFlag::UnorderedAccess | TextureFlag::ShaderResource,
			ClearBinding(Colors::Black),
			m_pSolidOutput->GetSampleCount());

		m_pIntermediateBlur = m_pDevice->CreateTexture(blurredColorTargetDesc, "Blurred Intermediate Color Target");
		m_pBlurredOutput = m_pDevice->CreateTexture(blurredColorTargetDesc, "Blurred Color Target");

		commandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
		commandContext.SetPipelineState(m_pGaussianBlurPSO);

		struct
		{
			uint32 SourceIndex;
			uint32 TargetIndex;
			uint32 Radius;
			uint32 IsHorizontal;
		} params;

		params.SourceIndex = m_pSolidOutput->GetSRVIndex();
		params.TargetIndex = m_pIntermediateBlur->GetUAVIndex();
		params.Radius = RADIUS;
		params.IsHorizontal = 1u;

		commandContext.BindRootCBV(BindingSlot::PerInstance, &m_CBData, sizeof(GaussianBlurCB));
		commandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(commandContext, renderView);

		commandContext.Dispatch(ComputeUtils::GetNumThreadGroups(m_pBlurredOutput->GetWidth(), 16, m_pBlurredOutput->GetHeight(), 16));
	
		commandContext.InsertUAVBarrier();

		params.SourceIndex = m_pIntermediateBlur->GetSRVIndex();
		params.TargetIndex = m_pBlurredOutput->GetUAVIndex();
		params.IsHorizontal = 0;
		
		commandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));

		commandContext.Dispatch(ComputeUtils::GetNumThreadGroups(m_pBlurredOutput->GetWidth(), 16, m_pBlurredOutput->GetHeight(), 16));

		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
	}

	Ref<TextureEx> Outlines::GetSelectedEntityIDOutput() const noexcept
	{
		return m_pSolidOutput;
	}

	Ref<TextureEx> Outlines::GetBlurredOutput() const noexcept
	{
		return m_pBlurredOutput;
	}

}