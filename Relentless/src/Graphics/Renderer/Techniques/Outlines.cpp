#include "Outlines.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/PipelineState.h"

#include "Scene/Scene.h"

namespace Relentless
{
	constexpr uint32 RADIUS = 3u;
	
	Outlines::Outlines(GraphicsDevice* aGraphicsDevice) noexcept
		: m_pDevice{ aGraphicsDevice }
	{
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
			m_CBData.Weights[i].value /= sum;
	}

	void Outlines::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures) noexcept
	{
		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesSolidTarget, D3D12_RESOURCE_STATE_RENDER_TARGET);

		RenderPassInfo info;
		info.RenderTargets[0].pTarget = aSceneTextures.pOutlinesSolidTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Clear;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;
		
		info.DepthStencilTarget.pTarget = aSceneTextures.pOutlinesDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;
		
		aCommandContext.BeginRenderPass(info);
		
		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		aCommandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Outlines - Solid");
		psoDesc.SetBlendMode(BlendMode::Replace);
		psoDesc.SetDepthEnabled(true);
		psoDesc.SetDepthWrite(true);
		psoDesc.SetDepthFunc(D3D12_COMPARISON_FUNC_LESS_EQUAL);
		psoDesc.SetVertexShader("EntityOutputShader", "vs_main");
		psoDesc.SetPixelShader("EntityOutputShader", "ps_main");
		psoDesc.SetRootSignature(m_pDevice->GetGlobalRootSignature());
		psoDesc.SetRenderTargetFormats(ResourceFormat::R32_FLOAT, ResourceFormat::D32_FLOAT, 1);

		aCommandContext.SetPipelineState(m_pDevice->GetOrCreatePipeline(psoDesc));
		
		Renderer::BindViewData(aCommandContext, aRenderView);
		
		EntityManager& entityManager = aRenderView.pScene->GetEntityManager();
		
		auto batches = aRenderView.pRenderScene->GetBatches();
		for (const Batch& batch : batches)
		{
			if (!entityManager.Exists(batch.EntityID)) //TODO, decouple from game thread...
				continue;

			if (entityManager.Has<SelectedInEditorComponent>(batch.EntityID))
			{
				struct
				{
					uint32 InstanceIndex;
					uint32 EntityID;
				} params;
		
				params.InstanceIndex = batch.InstanceID;
				params.EntityID = entityManager.GetIdentity(batch.EntityID) + 1;
		
				aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&params, sizeof(params));
				aCommandContext.Draw(0u, batch.NumIndices, 0u, 1u);
			}
		}
		
		aCommandContext.EndRenderPass();

		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesSolidTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT));

		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesIntermediateBlurTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesBlurTarget, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

		aCommandContext.SetComputeRootSignature(m_pDevice->GetGlobalRootSignature());
		aCommandContext.SetPipelineState(m_pDevice->GetOrCreateComputePipeline(m_pDevice->GetGlobalRootSignature(), "GaussianBlurSeparableShader", "cs_main"));

		struct
		{
			uint32 SourceIndex;
			uint32 TargetIndex;
			uint32 Radius;
			uint32 IsHorizontal;
		} params;

		params.SourceIndex = aSceneTextures.pOutlinesSolidTarget->GetSRVIndex();
		params.TargetIndex = aSceneTextures.pOutlinesIntermediateBlurTarget->GetUAVIndex();
		params.Radius = RADIUS;
		params.IsHorizontal = 1u;

		aCommandContext.BindRootCBV(BindingSlot::PerInstance, &m_CBData, sizeof(GaussianBlurCB));
		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));
		Renderer::BindViewData(aCommandContext, aRenderView);

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aSceneTextures.pOutlinesBlurTarget->GetWidth(), 16, aSceneTextures.pOutlinesBlurTarget->GetHeight(), 16));
	
		aCommandContext.InsertUAVBarrier();
		aCommandContext.InsertResourceBarrier(aSceneTextures.pOutlinesIntermediateBlurTarget, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

		params.SourceIndex = aSceneTextures.pOutlinesIntermediateBlurTarget->GetSRVIndex();
		params.TargetIndex = aSceneTextures.pOutlinesBlurTarget->GetUAVIndex();
		params.IsHorizontal = 0;
		
		aCommandContext.BindRootCBV(BindingSlot::PerPass, &params, sizeof(params));

		aCommandContext.Dispatch(ComputeUtils::GetNumThreadGroups(aSceneTextures.pOutlinesBlurTarget->GetWidth(), 16, aSceneTextures.pOutlinesBlurTarget->GetHeight(), 16));

		m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT)->InsertWait(m_pDevice->GetCommandQueue(D3D12_COMMAND_LIST_TYPE_COMPUTE));
	}
}