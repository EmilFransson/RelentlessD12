#include "CommandContext.h"

#include "Buffer.h"
#include "Device.h"
#include "PipelineState.h"
#include "ResourceViews.h"
#include "RootSignature.h"
#include "TextureEx.h"

namespace Relentless
{
	CommandContext::CommandContext(GraphicsDevice* pParent, Ref<ID3D12CommandList> pCommandList, Ref<ScratchAllocationManager> pScratchAllocationManager, D3D12_COMMAND_LIST_TYPE type) noexcept
		: 
		DeviceObject{pParent},
		m_ScratchAllocator{pScratchAllocationManager},
		m_Type{type}
	{
		RLS_VERIFY(pCommandList.As(&m_pCommandList), "Unable to promote command list.");
	}

	void CommandContext::AddBarrier(const D3D12_RESOURCE_BARRIER& barrier) noexcept
	{
		m_BatchedBarriers[m_NrOfBatchedBarriers++] = barrier;
		if (m_NrOfBatchedBarriers >= MAX_BATCHED_BARRIER_COUNT)
			FlushResourceBarriers();
	}

	ScratchAllocation CommandContext::AllocateScratch(uint64 size, uint32 alignment /*= 16*/) noexcept
	{
		return m_ScratchAllocator.Allocate(size, alignment);
	}

	void CommandContext::BindRootCBV(uint32 rootIndex, const void* pData, uint32 dataSize) noexcept
	{
		ScratchAllocation allocation = AllocateScratch(dataSize, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT);
		memcpy(allocation.pMappedMemory, pData, dataSize);

		if (m_CurrentCommandContext == CommandListContext::Graphics)
			m_pCommandList->SetGraphicsRootConstantBufferView(rootIndex, allocation.GpuHandle);
		else
			m_pCommandList->SetComputeRootConstantBufferView(rootIndex, allocation.GpuHandle);
	}

	void CommandContext::BindRootCBV(uint32 rootIndex, const BufferEx* pBuffer) noexcept
	{
		if (m_CurrentCommandContext == CommandListContext::Graphics)
			m_pCommandList->SetGraphicsRootConstantBufferView(rootIndex, pBuffer->GetGpuHandle());
		else
			m_pCommandList->SetComputeRootConstantBufferView(rootIndex, pBuffer->GetGpuHandle());
	}

	void CommandContext::CopyBuffer(const BufferEx* pSource, const BufferEx* pTarget, uint64 srcOffset, uint64 dstOffset, uint64 nrOfBytes) noexcept
	{
		RLS_ASSERT(pSource && pSource->GetResource(), "[CommandContext::CopyBuffer] Source is invalid.");
		RLS_ASSERT(pTarget && pTarget->GetResource(), "[CommandContext::CopyBuffer] Target is invalid.");

		FlushResourceBarriers();
		m_pCommandList->CopyBufferRegion(pTarget->GetResource(), dstOffset, pSource->GetResource(), srcOffset, nrOfBytes);
	}

	void CommandContext::BeginRenderPass(const RenderPassInfo& renderPassInfo) noexcept
	{
		RLS_ASSERT(!m_InRenderPass, "[CommandContext::BeginRenderPass] Already In Render Pass.");

		FlushResourceBarriers();
		
		std::array<D3D12_RENDER_PASS_RENDER_TARGET_DESC, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> renderTargets = {};

		for (uint8 i = 0u; i < renderPassInfo.RenderTargetCount; ++i)
		{
			const RenderPassInfo::RenderTargetInfo& renderTargetInfo = renderPassInfo.RenderTargets[i];
			RLS_ASSERT(renderTargetInfo.pTarget, "[CommandContext::BeginRenderPass] Render Target is invalid.");

			const TextureDesc& desc = renderTargetInfo.pTarget->GetDesc();
			switch (desc.Type)
			{
			case TextureType::Texture2D:
			{
				for (uint8 mip = 0u; mip < desc.Mips; ++mip)
					renderTargetInfo.pTarget->SetRTV(GetParent()->CreateRTV(renderTargetInfo.pTarget, TextureRTVDesc(mip, 0u, 1u, 0u)), mip);
				break;
			}
			case TextureType::TextureCube:
			{
				for (uint8 face = 0u; face < 6; ++face)
				{
					for (uint8 mip = 0u; mip < desc.Mips; ++mip)
						renderTargetInfo.pTarget->SetRTV(GetParent()->CreateRTV(renderTargetInfo.pTarget, TextureRTVDesc(mip, face, 1u, 0u)), (face * desc.Mips) + mip);
				}
				break;
			}
			default:
				RLS_ASSERT(false, "Unreachable.");
				break;
			}

			D3D12_RENDER_PASS_RENDER_TARGET_DESC& renderTargetDesc = renderTargets[i];
			renderTargetDesc.cpuDescriptor = renderTargetInfo.pTarget->GetRTV()->GetDescriptorHandle().CPUHandle;

			if (EnumHasAnyFlags(renderTargetInfo.BeginAccessFlags, RenderTargetAccessFlags::Preserve))
				renderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
			else if (EnumHasAnyFlags(renderTargetInfo.BeginAccessFlags, RenderTargetAccessFlags::Clear))
				renderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
			else if (EnumHasAnyFlags(renderTargetInfo.BeginAccessFlags, RenderTargetAccessFlags::Discard))
				renderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
			else if (EnumHasAnyFlags(renderTargetInfo.BeginAccessFlags, RenderTargetAccessFlags::None))
				renderTargetDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;

			if (EnumHasAnyFlags(renderTargetInfo.EndAccessFlags, RenderTargetAccessFlags::Preserve))
				renderTargetDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
			else if (EnumHasAnyFlags(renderTargetInfo.EndAccessFlags, RenderTargetAccessFlags::Discard))
				renderTargetDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD;
			else if (EnumHasAnyFlags(renderTargetInfo.EndAccessFlags, RenderTargetAccessFlags::Resolve))
			{
				RLS_ASSERT(renderTargetInfo.pResolveTarget && renderTargetInfo.pResolveTarget->GetResource(), "[CommandContext::BeginRenderPass] Resolve Target Is Invalid.");
				RLS_ASSERT(renderTargetInfo.pTarget->GetFormat() == renderTargetInfo.pResolveTarget->GetFormat(), "[CommandContext::BeginRenderPass] Src & Resolve Dst Format Mismatch.");
				RLS_ASSERT(renderTargetInfo.pTarget->GetSampleCount() > 1, "[CommandContext::BeginRenderPass] Resolve Src Is Not MultiSample Compatible.");
				RLS_ASSERT(renderTargetInfo.pResolveTarget->GetSampleCount() == 1, "[CommandContext::BeginRenderPass] Resolve Target Sample Count Exceeds 1.");

				renderTargetDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_RESOLVE;

				auto& resolveParams = renderTargetDesc.EndingAccess.Resolve;
				resolveParams.pSrcResource = renderTargetInfo.pTarget->GetResource();
				resolveParams.pDstResource = renderTargetInfo.pResolveTarget->GetResource();
				resolveParams.Format = D3D::ConvertFormat(renderTargetInfo.pTarget->GetFormat());
				resolveParams.ResolveMode = D3D12_RESOLVE_MODE_AVERAGE;
				resolveParams.SubresourceCount = 1;
				
				D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS resolveSubresParams = {};
				resolveSubresParams.SrcSubresource = 0; 
				resolveSubresParams.DstSubresource = 0; 
				resolveSubresParams.SrcRect = { 0, 0, static_cast<long>(renderTargetInfo.pTarget->GetWidth()), static_cast<long>(renderTargetInfo.pTarget->GetHeight())};
				resolveSubresParams.DstX = resolveSubresParams.DstY = 0;
				resolveParams.pSubresourceParameters = &resolveSubresParams;
			}
			else if (EnumHasAnyFlags(renderTargetInfo.EndAccessFlags, RenderTargetAccessFlags::None))
				renderTargetDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

			if(EnumHasAnyFlags(renderPassInfo.RenderTargets[i].BeginAccessFlags, RenderTargetAccessFlags::Clear))
			{
				const ClearBinding& clearBinding = renderTargetInfo.pTarget->GetClearBinding();
				renderTargetDesc.BeginningAccess.Clear.ClearValue.Format = D3D::ConvertFormat(renderTargetInfo.pTarget->GetFormat());
				renderTargetDesc.BeginningAccess.Clear.ClearValue.Color[0] = clearBinding.Color.R();
				renderTargetDesc.BeginningAccess.Clear.ClearValue.Color[1] = clearBinding.Color.G();
				renderTargetDesc.BeginningAccess.Clear.ClearValue.Color[2] = clearBinding.Color.B();
				renderTargetDesc.BeginningAccess.Clear.ClearValue.Color[3] = clearBinding.Color.A();
			}
		}

		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pDepthStencilDesc = nullptr;
		D3D12_RENDER_PASS_DEPTH_STENCIL_DESC depthStencilDesc;
		if (renderPassInfo.DepthStencilTarget.pTarget)
		{
			const RenderPassInfo::DepthTargetInfo& depthStencilTarget = renderPassInfo.DepthStencilTarget;

			const TextureDesc& desc = depthStencilTarget.pTarget->GetDesc();

			switch (desc.Type)
			{
			case TextureType::Texture2D:
			{
				for (uint8 mip = 0u; mip < desc.Mips; ++mip)
					depthStencilTarget.pTarget->SetDSV(GetParent()->CreateDSV(depthStencilTarget.pTarget, TextureDSVDesc(depthStencilTarget.BeginAccessFlags, mip)), mip);
				break;
			}
			case TextureType::TextureCube:
			{
				for (uint8 face = 0u; face < 6; ++face)
				{
					for (uint8 mip = 0u; mip < desc.Mips; ++mip)
						depthStencilTarget.pTarget->SetDSV(GetParent()->CreateDSV(depthStencilTarget.pTarget, depthStencilTarget.BeginAccessFlags));
				}
				break;
			}
			default:
				RLS_ASSERT(false, "Unreachable.");
				break;
			}

			depthStencilDesc.cpuDescriptor = depthStencilTarget.pTarget->GetDSV()->GetDescriptorHandle().CPUHandle;

			if (EnumHasAnyFlags(depthStencilTarget.BeginAccessFlags, DepthTargetAccessFlags::ClearDepth))
			{
				const ClearBinding& clearBinding = depthStencilTarget.pTarget->GetClearBinding();
				depthStencilDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
				depthStencilDesc.DepthBeginningAccess.Clear.ClearValue.Format = D3D::ConvertFormat(depthStencilTarget.pTarget->GetFormat());
				depthStencilDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth = clearBinding.DepthStencil.Depth;
			}
			else if (EnumHasAnyFlags(depthStencilTarget.BeginAccessFlags, DepthTargetAccessFlags::ReadOnlyDepth))
				depthStencilDesc.DepthBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;
			
			if (EnumHasAnyFlags(depthStencilTarget.BeginAccessFlags, DepthTargetAccessFlags::ClearStencil))
			{ 
				const ClearBinding& clearBinding = depthStencilTarget.pTarget->GetClearBinding();
				depthStencilDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR;
				depthStencilDesc.DepthBeginningAccess.Clear.ClearValue.Format = D3D::ConvertFormat(depthStencilTarget.pTarget->GetFormat());
				depthStencilDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Stencil = clearBinding.DepthStencil.Stencil;
			}
			else if (EnumHasAnyFlags(depthStencilTarget.BeginAccessFlags, DepthTargetAccessFlags::ReadOnlyStencil))
				depthStencilDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE;

			depthStencilDesc.DepthEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE;
			depthStencilDesc.StencilBeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
			depthStencilDesc.StencilEndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

			pDepthStencilDesc = &depthStencilDesc;
		}

		//m_pCommandList->ClearRenderTargetView(renderPassInfo.RenderTargets[0].pTarget->GetRTV()->GetDescriptorHandle().CPUHandle,
		//	renderPassInfo.RenderTargets[0].pTarget->GetClearBinding().Color, 0, nullptr);
		//
		//m_pCommandList->ClearDepthStencilView(renderPassInfo.DepthStencilTarget.pTarget->GetDSV()->GetDescriptorHandle().CPUHandle,
		//	D3D12_CLEAR_FLAGS::D3D12_CLEAR_FLAG_DEPTH, renderPassInfo.DepthStencilTarget.pTarget->GetClearBinding().DepthStencil.Depth, renderPassInfo.DepthStencilTarget.pTarget->GetClearBinding().DepthStencil.Stencil, 0,nullptr);

		m_pCommandList->BeginRenderPass(renderPassInfo.RenderTargetCount, renderTargets.data(), pDepthStencilDesc, D3D12_RENDER_PASS_FLAG_NONE);

		TextureEx* pTargetTexture = renderPassInfo.DepthStencilTarget.pTarget ? renderPassInfo.DepthStencilTarget.pTarget : renderPassInfo.RenderTargets[0].pTarget;
		SetViewport(FloatRect(0, 0, (float)pTargetTexture->GetWidth(), (float)pTargetTexture->GetHeight()), 0, 1);

		m_CurrentRenderPassInfo = renderPassInfo;
		m_InRenderPass = true;
	}

	void CommandContext::ClearState() noexcept
	{
		if (m_Type != D3D12_COMMAND_LIST_TYPE_COPY)
		{
			FlushResourceBarriers();

			m_CurrentCommandContext = CommandListContext::Invalid;

			m_pCurrentPSO = nullptr;
			m_pCurrentGraphicsRS = nullptr;
			m_pCurrentComputeRS = nullptr;

			m_pCommandList->ClearState(nullptr);


			ID3D12DescriptorHeap* pHeaps[] =
			{
				GetParent()->GetGlobalShaderBindableHeap()->GetDescriptorHeapInterface(),
				GetParent()->GetGlobalSamplerHeap()->GetDescriptorHeapInterface()
			};

			m_pCommandList->SetDescriptorHeaps(2u, pHeaps);
		}
	}

	void CommandContext::CopyResource(const DeviceResource* pSource, const DeviceResource* pTarget) noexcept
	{
		RLS_ASSERT(pSource && pSource->GetResource(), "[CommandContext::CopyResource] Source is invalid.");
		RLS_ASSERT(pTarget && pTarget->GetResource(), "[CommandContext::CopyResource] Target is invalid.");

		FlushResourceBarriers();
		m_pCommandList->CopyResource(pTarget->GetResource(), pSource->GetResource());
	}

	void CommandContext::CopyTexture(const TextureEx* pSource, const BufferEx* pTarget, const D3D12_BOX& sourceRegion, uint32 sourceSubresource /*= 0*/, uint32 destinationOffset /*= 0*/) noexcept
	{
		RLS_ASSERT(pSource && pSource->GetResource(), "[CommandContext::CopyTexture] Source is invalid.");
		RLS_ASSERT(pTarget && pTarget->GetResource(), "[CommandContext::CopyTexture] Target is invalid.");
		RLS_ASSERT(destinationOffset % D3D12_TEXTURE_DATA_PLACEMENT_ALIGNMENT == 0, "[CommandContext::CopyTexture] Destination offset must be aligned.");

		D3D12_PLACED_SUBRESOURCE_FOOTPRINT textureFootprint;
		textureFootprint.Offset = 0;
		textureFootprint.Footprint.Width = sourceRegion.right - sourceRegion.left;
		textureFootprint.Footprint.Depth = sourceRegion.back - sourceRegion.front;
		textureFootprint.Footprint.Height = sourceRegion.bottom - sourceRegion.top;
		textureFootprint.Footprint.Format = D3D::ConvertFormat(pSource->GetFormat());
		textureFootprint.Footprint.RowPitch = Math::AlignUp<uint32>((uint32)RHI::GetRowPitch(pSource->GetFormat(), textureFootprint.Footprint.Width), D3D12_TEXTURE_DATA_PITCH_ALIGNMENT);

		CD3DX12_TEXTURE_COPY_LOCATION srcLocation(pSource->GetResource(), sourceSubresource);
		CD3DX12_TEXTURE_COPY_LOCATION dstLocation(pTarget->GetResource(), textureFootprint);
		FlushResourceBarriers();
		m_pCommandList->CopyTextureRegion(&dstLocation, destinationOffset, 0, 0, &srcLocation, &sourceRegion);
	}

	void CommandContext::CopyTexture(const TextureEx* pSource, const TextureEx* pTarget, const D3D12_BOX& sourceRegion, const D3D12_BOX& destinationRegion, uint32 sourceSubresource, uint32 destinationSubresource) noexcept
	{
		RLS_ASSERT(pSource && pSource->GetResource(), "[CommandContext::CopyTexture] Source is invalid.");
		RLS_ASSERT(pTarget && pTarget->GetResource(), "[CommandContext::CopyTexture] Target is invalid.");

		CD3DX12_TEXTURE_COPY_LOCATION srcLocation(pSource->GetResource(), sourceSubresource);
		CD3DX12_TEXTURE_COPY_LOCATION dstLocation(pTarget->GetResource(), destinationSubresource);
		FlushResourceBarriers();
		m_pCommandList->CopyTextureRegion(&dstLocation, destinationRegion.left, destinationRegion.top, destinationRegion.front, &srcLocation, &sourceRegion);
	}

	void CommandContext::Dispatch(uint32 groupCountX, uint32 groupCountY /*= 1*/, uint32 groupCountZ /*= 1*/) noexcept
	{
		RLS_ASSERT(m_CurrentCommandContext == CommandListContext::Compute, "[CommandContext::Dispatch] Invalid Command List Context");
		RLS_ASSERT(m_pCurrentPSO, "[CommandContext::Dispatch] No/Invalid Pipeline State Object");
		RLS_ASSERT(
			groupCountX <= D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION &&
			groupCountY <= D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION &&
			groupCountZ <= D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION,
			"Dispatch Group Size ({0} x {1} x {2}) Can Not Exceed {3}", groupCountX, groupCountY, groupCountZ, D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION);

		PrepareDraw();
		if (groupCountX > 0 && groupCountY > 0 && groupCountZ > 0)
			m_pCommandList->Dispatch(groupCountX, groupCountY, groupCountZ);
	}

	void CommandContext::Dispatch(const Vector3i groupCount) noexcept
	{
		Dispatch(groupCount.x, groupCount.y, groupCount.z);
	}

	void CommandContext::InsertResourceBarrier(DeviceResource* pResource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, uint32 subResource /*= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES*/) noexcept
	{
		RLS_ASSERT(!m_InRenderPass, "[CommandContext::InsertResourceBarrier] Inserting Barriers Is Disallowed While In A Render Pass.");
		RLS_ASSERT(pResource && pResource->GetResource(), "[CommandContext::InsertResourceBarrier] Resource Is Invalid.");
		RLS_ASSERT(pResource->UsesStateTracking(), "[CommandContext::InsertResourceBarrier] Resource Does Not Use State Tracking.");
		RLS_ASSERT(D3D::IsTransitionAllowed(m_Type, beforeState), "[CommandContext::InsertResourceBarrier] Before State Is Not Valid On This Commandlist Type ({0})", D3D::CommandListTypeToString(m_Type));
		RLS_ASSERT(D3D::IsTransitionAllowed(m_Type, afterState), "[CommandContext::InsertResourceBarrier] After State Is Not Valid On This Commandlist Type ({0})", D3D::CommandListTypeToString(m_Type));

		ResourceState& localResourceState = m_ResourceStates[pResource];
		D3D12_RESOURCE_STATES localBeforeState = localResourceState.Get(subResource);
		RLS_ASSERT(beforeState == D3D12_RESOURCE_STATE_UNKNOWN || localBeforeState == D3D12_RESOURCE_STATE_UNKNOWN || localBeforeState == beforeState, "Provided before state of resource {0} does not match with tracked resource state", pResource->GetName());

		// If the given before state is "Unknown", get it from the commandlist
		if (beforeState == D3D12_RESOURCE_STATE_UNKNOWN)
			beforeState = localBeforeState;

		if (beforeState == D3D12_RESOURCE_STATE_UNKNOWN)
		{
			localResourceState.Set(afterState, subResource);

			PendingBarrier& barrier = m_PendingBarriers.emplace_back();
			barrier.pResource = pResource;
			barrier.State = afterState;
			barrier.Subresource = subResource;
		}
		else
		{
			if (D3D::NeedsTransition(beforeState, afterState, true))
			{
				if (m_NrOfBatchedBarriers > 0)
				{
					// If the previous barrier is for the same resource, see if we can combine the barrier.
					D3D12_RESOURCE_BARRIER& last = m_BatchedBarriers[m_NrOfBatchedBarriers - 1];
					if (last.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION
						&& last.Transition.pResource == pResource->GetResource()
						&& last.Transition.StateBefore == beforeState
						&& D3D::CanCombineResourceState(afterState, last.Transition.StateAfter))
					{
						last.Transition.StateAfter |= afterState;
						return;
					}
				}
				AddBarrier(CD3DX12_RESOURCE_BARRIER::Transition(pResource->GetResource(),
					beforeState,
					afterState,
					subResource,
					D3D12_RESOURCE_BARRIER_FLAG_NONE)
				);

				localResourceState.Set(afterState, subResource);
			}
		}
	}

	void CommandContext::InsertUAVBarrier(const DeviceResource* pResource /*= nullptr*/) noexcept
	{
		AddBarrier(CD3DX12_RESOURCE_BARRIER::UAV(pResource ? pResource->GetResource() : nullptr));
	}

	void CommandContext::Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceStart, uint32_t instanceCount) noexcept
	{
		RLS_ASSERT(m_CurrentCommandContext == CommandListContext::Graphics, "[CommandContext] Invalid Command List Context");
		RLS_ASSERT(m_pCurrentPSO, "[CommandContext] No/Invalid Pipeline State Object");

		PrepareDraw();
		m_pCommandList->DrawInstanced(vertexCount, instanceCount, vertexStart, instanceStart);
	}

	void CommandContext::DrawIndexedInstanced(uint32_t indexCount, uint32_t indexStart, uint32_t instances, uint32_t instanceStart, int vertexBaseLocation) noexcept
	{
		RLS_ASSERT(m_CurrentCommandContext == CommandListContext::Graphics, "[CommandContext] Invalid Command List Context");
		RLS_ASSERT(m_pCurrentPSO, "[CommandContext] No/Invalid Pipeline State Object");

		PrepareDraw();
		m_pCommandList->DrawIndexedInstanced(indexCount, instances, indexStart, vertexBaseLocation, instanceStart);
	}

	void CommandContext::EndRenderPass() noexcept
	{
		RLS_ASSERT(m_InRenderPass, "[CommandContext::EndRenderPass] Context Is Not In Render Pass.");
		
		m_pCommandList->EndRenderPass();
		m_InRenderPass = false;
	}

	SyncPoint CommandContext::Execute() noexcept
	{
		return CommandContext::Execute({ this });
	}

	SyncPoint CommandContext::Execute(Span<CommandContext* const> commandContexts) noexcept
	{
		RLS_ASSERT(commandContexts.GetSize() > 0, "[CommandContext::Execute] No Command Contexts Available For Execution.");
		CommandQueue* pQueue = commandContexts[0]->GetParent()->GetCommandQueue(commandContexts[0]->GetType());

		for (CommandContext* pCommandContext : commandContexts)
		{
			RLS_ASSERT(pCommandContext->GetType() == pQueue->GetType(), "[CommandContext::Execute] Mismatching Command Context and Command Queue Types.");
			pCommandContext->FlushResourceBarriers();
		}

		SyncPoint syncPoint = pQueue->ExecuteCommandLists(commandContexts);
		
		for (CommandContext* pContext : commandContexts)
			pContext->Free(syncPoint);

		return syncPoint;
	}

	void CommandContext::Free(const SyncPoint& syncPoint)
	{
		m_ScratchAllocator.Free(syncPoint);

		GetParent()->GetCommandQueue(m_Type)->FreeAllocator(syncPoint, m_pCommandAllocator);
		m_pCommandAllocator = nullptr;
		GetParent()->FreeCommandContext(this);
	}

	ID3D12GraphicsCommandListX* CommandContext::GetCommandList() const noexcept
	{
		return m_pCommandList;
	}

	D3D12_COMMAND_LIST_TYPE CommandContext::GetType() const noexcept
	{
		return m_Type;
	}

	void CommandContext::Reset() noexcept
	{
		RLS_ASSERT(m_pCommandList, "[CommandContext::Reset] Command List Is Invalid.");

		if (m_pCommandAllocator == nullptr)
		{
			m_pCommandAllocator = GetParent()->GetCommandQueue(m_Type)->RequestAllocator();
			VERIFY_HR_EX(m_pCommandList->Reset(m_pCommandAllocator, nullptr), GetParent()->GetDevice());
		}

		RLS_ASSERT(m_NrOfBatchedBarriers == 0, "[CommandContext::Reset] Command Context Has Unreseolved Barriers.");
		RLS_ASSERT(m_PendingBarriers.empty(), "[CommandContext::Reset] Command Context Has Unreseolved Barriers.");
		m_ResourceStates.clear();
	
		ClearState();
	}

	void CommandContext::ResolvePendingBarriers(CommandContext& resolveContext)
	{
		if (m_PendingBarriers.empty())
			return;

		for (const PendingBarrier& pending : m_PendingBarriers)
		{
			const uint32 subResource = pending.Subresource;
			DeviceResource* pResource = pending.pResource;

			// Retrieve the last known resource state
			const D3D12_RESOURCE_STATES beforeState = pResource->GetResourceState(subResource);
			RLS_ASSERT(D3D::IsTransitionAllowed(m_Type, beforeState),
				"[ommandContext::ResolvePendingBarriers] Resource ({0}) Can Not Be Transitioned From This State On This Queue ({1}). Insert A Barrier On Another Queue Before Executing This One.",
				pResource->GetName(), D3D::CommandListTypeToString(m_Type));

			// Get the after state of the first use in the current cmdlist
			D3D12_RESOURCE_STATES afterState = pending.State;
			if (D3D::NeedsTransition(beforeState, afterState, false))
				resolveContext.AddBarrier(CD3DX12_RESOURCE_BARRIER::Transition(pResource->GetResource(), beforeState, afterState, subResource));

			// Update the resource with the last known state of the current cmdlist
			const D3D12_RESOURCE_STATES end_state = GetLocalResourceState(pending.pResource, subResource);
			pResource->SetResourceState(end_state, subResource);
		}

		resolveContext.FlushResourceBarriers();
		m_PendingBarriers.clear();
	}

	void CommandContext::ResolveResource(TextureEx* pSource, uint32 sourceSubResource, TextureEx* pTarget, uint32 targetSubResource, ResourceFormat format) noexcept
	{
		FlushResourceBarriers();
		m_pCommandList->ResolveSubresource(pTarget->GetResource(), targetSubResource, pSource->GetResource(), sourceSubResource, D3D::ConvertFormat(format));
	}

	void CommandContext::SetGraphicsRootSignature(RootSignature* pRootSignature) noexcept
	{
		m_pCommandList->SetGraphicsRootSignature(pRootSignature->GetRootSignature());
		m_CurrentCommandContext = CommandListContext::Graphics;
		m_pCurrentGraphicsRS = pRootSignature;
	}

	void CommandContext::SetComputeRootSignature(RootSignature* pRootSignature) noexcept
	{
		m_CurrentCommandContext = CommandListContext::Compute;
		if (pRootSignature != m_pCurrentComputeRS)
		{
			m_pCommandList->SetComputeRootSignature(pRootSignature->GetRootSignature());
			m_pCurrentComputeRS = pRootSignature;
		}
	}

	void CommandContext::SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) noexcept
	{
		m_pCommandList->IASetPrimitiveTopology(topology);
	}

	void CommandContext::SetPipelineState(PipelineState* pPipeline) noexcept
	{
		RLS_ASSERT(pPipeline, "[CommandContext] Pipeline is invalid.");
		pPipeline->ConditionallyReload();

		m_pCommandList->SetPipelineState(pPipeline->GetPipelineState());
		m_pCurrentPSO = pPipeline;
	}

	void CommandContext::SetScissorRect(const FloatRect& rect) noexcept
	{
		const D3D12_RECT scissorRect
		{
			.left	= (LONG)rect.Left,
			.top	= (LONG)rect.Top,
			.right	= (LONG)rect.Right,
			.bottom = (LONG)rect.Bottom,
		};

		m_pCommandList->RSSetScissorRects(1u, &scissorRect);
	}

	void CommandContext::SetViewport(const FloatRect& rect, float minDepth, float maxDepth) noexcept
	{
		D3D12_VIEWPORT viewport
		{
			.TopLeftX = rect.Left,
			.TopLeftY = rect.Top,
			.Width = rect.GetWidth(),
			.Height = rect.GetHeight(),
			.MinDepth = minDepth,
			.MaxDepth = maxDepth
		};

		m_pCommandList->RSSetViewports(1u, &viewport);
		SetScissorRect(rect);
	}

	void CommandContext::FlushResourceBarriers() noexcept
	{
		if (m_NrOfBatchedBarriers > 0u)
		{
			m_pCommandList->ResourceBarrier(m_NrOfBatchedBarriers, m_BatchedBarriers.data());
			m_NrOfBatchedBarriers = 0u;
		}
	}

	D3D12_RESOURCE_STATES CommandContext::GetLocalResourceState(const DeviceResource* pResource, uint32 subResource) const
	{
		auto it = m_ResourceStates.find(pResource);
		RLS_ASSERT(it != m_ResourceStates.end(), "[CommandContext::GetLocalResourceState] Local Resource State Does Not Exist.");
		return it->second.Get(subResource);
	}

	void CommandContext::PrepareDraw() noexcept
	{
		RLS_ASSERT(m_CurrentCommandContext != CommandListContext::Invalid, "[CommandContext] Commandlist Context Is Invalid");
		FlushResourceBarriers();
	}

}
