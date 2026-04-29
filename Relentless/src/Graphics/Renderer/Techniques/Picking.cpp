#include "Picking.h"

#include "Graphics/Renderer/Renderer.h"
#include "Graphics/RHI/Device.h"
#include "Graphics/RHI/CommandContext.h"
#include "Graphics/RHI/PipelineState.h"

#include "Subsystem/CoreTypes/MeshRenderSubsystem.h"
#include "Subsystem/CoreTypes/PrimitiveRenderSubsystem.h"

namespace Relentless
{
	Picking::Picking(GraphicsDevice* aDevice) noexcept
		: m_pDevice{ aDevice }
	{
	}

	void Picking::Render(CommandContext& aCommandContext, const RenderView& aRenderView, SceneTextures& aSceneTextures, SceneBuffers& aSceneBuffers) noexcept
	{
		RenderPassInfo info;
		info.RenderTargets[0].pTarget = aSceneTextures.pEntityIDTarget;
		info.RenderTargets[0].BeginAccessFlags = RenderTargetAccessFlags::Clear;
		info.RenderTargets[0].EndAccessFlags = RenderTargetAccessFlags::Preserve;
		info.RenderTargetCount++;

		info.DepthStencilTarget.pTarget = aSceneTextures.pEntityDepthTarget;
		info.DepthStencilTarget.BeginAccessFlags = DepthTargetAccessFlags::ClearDepth;
		info.DepthStencilTarget.EndAccessFlags = DepthTargetAccessFlags::None;

		aCommandContext.BeginRenderPass(info);

		aCommandContext.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		aCommandContext.SetGraphicsRootSignature(m_pDevice->GetGlobalRootSignature());

		PipelineStateInitializer psoDesc{};
		psoDesc.SetName("Picking");
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

		PrimitiveRenderSubsystem* pPrimitiveRenderSubsystem = aRenderView.pRenderScene->GetSubsystem<PrimitiveRenderSubsystem>();
		MeshRenderSubsystem* pMeshRenderSubsystem = aRenderView.pRenderScene->GetSubsystem<MeshRenderSubsystem>();

		for (const ShaderInterop::InstanceData& instanceData : pPrimitiveRenderSubsystem->GetInstanceCache())
		{
			const PrimitiveRenderProxy& primitiveRenderProxy = pPrimitiveRenderSubsystem->GetProxy(instanceData.EntityID);
			const uint32 numIndices = pMeshRenderSubsystem->GetProxy(primitiveRenderProxy.MeshUUID).IndexBuffer->GetNrOfElements();

			struct
			{
				uint32 InstanceIndex;
				uint32 EntityID;
			} params;

			params.InstanceIndex = instanceData.ID;
			params.EntityID = (instanceData.EntityID >> 12) + 1;

			aCommandContext.BindRootCBV(BindingSlot::PerInstance, (const void*)&params, sizeof(params));
			aCommandContext.Draw(0u, numIndices, 0u, 1u);
		}

		aCommandContext.EndRenderPass();

		const bool drawEntityMask = aRenderView.MouseHoverCoordinates.x >= 0 && aRenderView.MouseHoverCoordinates.y >= 0;
		if (drawEntityMask)
		{
			const uint32 x = Math::Min(aRenderView.MouseHoverCoordinates.x, (int)(aSceneTextures.pEntityIDTarget->GetWidth() - 1));
			const uint32 y = Math::Min(aRenderView.MouseHoverCoordinates.y, (int)(aSceneTextures.pEntityIDTarget->GetHeight() - 1));

			D3D12_BOX box{};
			box.left = x;
			box.right = box.left + 1;
			box.top = y;
			box.bottom = box.top + 1;
			box.front = 0;
			box.back = 1;

			aCommandContext.InsertResourceBarrier(aSceneTextures.pEntityIDTarget, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
			aCommandContext.CopyTexture(aSceneTextures.pEntityIDTarget, aSceneBuffers.EntityIDReadbackBuffer.pBuffer, box);
			aCommandContext.InsertResourceBarrier(aSceneTextures.pEntityIDTarget, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
		}
	}
}