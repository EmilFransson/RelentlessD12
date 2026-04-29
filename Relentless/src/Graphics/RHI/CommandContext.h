#pragma once

#include "Core/DLLExport.h"

#include "DeviceResource.h"

#include "Fence.h"

#include "ScratchAllocator.h"

namespace Relentless
{
	enum class CommandListContext : uint8_t
	{
		Graphics,
		Compute,
		Invalid
	};

	enum class RenderTargetAccessFlags : uint8
	{
		None		= 0,
		Preserve	= 1 << 0,
		Clear		= 1 << 1,
		Discard		= 1 << 2,
		Resolve		= 1 << 3
	};
	DEFINE_ENUM_FLAG_OPERATORS(RenderTargetAccessFlags)

	enum class DepthTargetAccessFlags : uint8
	{
		None			= 0,
		Preserve		= 1 << 0,
		ClearDepth		= 1 << 1,
		ClearStencil	= 1 << 2,
		
		ReadOnlyDepth	= 1 << 3,
		ReadOnlyStencil = 1 << 4,

		Resolve			= 1 << 5,

		ReadOnly		= ReadOnlyDepth | ReadOnlyStencil,
		Clear			= ClearDepth | ClearStencil
	};
	DEFINE_ENUM_FLAG_OPERATORS(DepthTargetAccessFlags)

	struct RenderPassInfo
	{
		struct RenderTargetInfo
		{
			Texture* pTarget = nullptr;
			Texture* pResolveTarget = nullptr;

			RenderTargetAccessFlags BeginAccessFlags = RenderTargetAccessFlags::None;
			RenderTargetAccessFlags EndAccessFlags = RenderTargetAccessFlags::None;

			uint8 MipLevel = 0;
			uint8 ArrayIndex = 0;
		};

		struct DepthTargetInfo
		{
			Texture* pTarget = nullptr;
			Texture* pResolveTarget = nullptr;

			DepthTargetAccessFlags BeginAccessFlags = DepthTargetAccessFlags::None;
			DepthTargetAccessFlags EndAccessFlags = DepthTargetAccessFlags::None;
		};

		RenderPassInfo() = default;

		RenderPassInfo(Texture* pRenderTarget, RenderTargetAccessFlags beginAccessFlags, RenderTargetAccessFlags endAccessFlags, Texture* pDepthTarget, DepthTargetAccessFlags beginDepthTargetFlags, DepthTargetAccessFlags endDepthTargetFlags) noexcept
			: RenderTargetCount{ 1u }
		{
			RenderTargets[0].pTarget = pRenderTarget;
			RenderTargets[0].BeginAccessFlags = beginAccessFlags;
			RenderTargets[0].EndAccessFlags = endAccessFlags;

			DepthStencilTarget.pTarget = pDepthTarget;
			DepthStencilTarget.BeginAccessFlags = beginDepthTargetFlags;
			DepthStencilTarget.EndAccessFlags = endDepthTargetFlags;
		}

		NO_DISCARD RenderTargetInfo& AddRenderTarget() noexcept
		{
			RLS_ASSERT(RenderTargetCount < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT, "[RenderPassInfo::AddRenderTarget]: Render target count exceeds maximum supported.");

			RenderTargetCount++;
			return RenderTargets[RenderTargetCount - 1];
		}

		NO_DISCARD static RenderPassInfo DepthOnly(Texture* pTarget, DepthTargetAccessFlags beginAccessFlags, DepthTargetAccessFlags endAccessFlags) noexcept
		{
			RenderPassInfo renderPassInfo;
			renderPassInfo.DepthStencilTarget.pTarget = pTarget;
			renderPassInfo.DepthStencilTarget.BeginAccessFlags = beginAccessFlags;
			renderPassInfo.DepthStencilTarget.EndAccessFlags = endAccessFlags;
			return renderPassInfo;
		}

		uint32 RenderTargetCount = 0;
		std::array<RenderTargetInfo, D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargets{};
		DepthTargetInfo DepthStencilTarget{};
	};

	class RLS_API CommandContext : public DeviceObject
	{
	public:
		CommandContext(GraphicsDevice* pParent, Ref<ID3D12CommandList> pCommandList, Ref<ScratchAllocationManager> pScratchAllocationManager, D3D12_COMMAND_LIST_TYPE type) noexcept;
		virtual ~CommandContext() noexcept = default;

		void AddBarrier(const D3D12_RESOURCE_BARRIER& barrier) noexcept;
		ScratchAllocation AllocateScratch(uint64 size, uint32 alignment = 16u) noexcept;
		
		void BindRootCBV(uint32 rootIndex, const void* pData, uint32 dataSize) noexcept;
		void BindRootCBV(uint32 rootIndex, const Buffer* allocation) noexcept;
		void BeginRenderPass(const RenderPassInfo& renderPassInfo) noexcept;
		
		void CopyBuffer(const Buffer* pSource, const Buffer* pTarget, uint64 size, uint64 sourceOffset, uint64 destinationOffset) noexcept;
		void ClearBufferUInt(const Buffer* pBuffer, uint32 value = 0);
		void ClearRenderTarget(const Texture* aTexture) noexcept;
		void ClearState() noexcept;
		void CopyResource(const DeviceResource* pSource, const DeviceResource* pTarget) noexcept;
		void CopyTexture(const Texture* pSource, const Buffer* pTarget, const D3D12_BOX& sourceRegion, uint32 sourceSubresource = 0u, uint32 destinationOffset = 0u) noexcept;
		void CopyTexture(const Texture* pSource, const Texture* pDestination, const D3D12_BOX& sourceRegion, const D3D12_BOX& destinationRegion, uint32 sourceSubresource = 0, uint32 destinationSubresource = 0) noexcept;
		
		void Dispatch(uint32 groupCountX, uint32 groupCountY = 1, uint32 groupCountZ = 1) noexcept;
		void Dispatch(const Vector3i groupCount) noexcept;
		
		void InsertResourceBarrier(DeviceResource* pResource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState, uint32 subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) noexcept;
		void InsertResourceBarrier(DeviceResource* pResource, D3D12_RESOURCE_STATES aNewState, uint32 subResource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES) noexcept;
		void InsertUAVBarrier(const DeviceResource* pResource = nullptr) noexcept;
		
		void Draw(uint32_t vertexStart, uint32_t vertexCount, uint32_t instanceStart, uint32_t instanceCount) noexcept;
		void DrawIndexedInstanced(uint32_t indexCount, uint32_t indexStart, uint32_t instances, uint32_t instanceStart, int vertexBaseLocation) noexcept;
		
		void EndRenderPass() noexcept;
		SyncPoint Execute() noexcept;
		static SyncPoint Execute(Span<CommandContext* const> pCommandContexts) noexcept;
		
		void Free(const SyncPoint& syncPoint);
		
		[[nodiscard]] ID3D12GraphicsCommandListX* GetCommandList() const noexcept;
		[[nodiscard]] D3D12_COMMAND_LIST_TYPE GetType() const noexcept;
		
		void Reset() noexcept;
		void ResolvePendingBarriers(CommandContext& resolveContext);
		void ResolveResource(Texture* pSource, uint32 sourceSubResource, Texture* pTarget, uint32 targetSubResource, ResourceFormat format) noexcept;

		void SetGraphicsRootSignature(RootSignature* pRootSignature) noexcept;
		void SetComputeRootSignature(RootSignature* pRootSignature) noexcept;
		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY topology) noexcept;
		void SetPipelineState(PipelineState* pPipeline) noexcept;
		void SetScissorRect(const FloatRect& rect) noexcept;
		void SetViewport(const FloatRect& rect, float minDepth, float maxDepth) noexcept;
		void FlushResourceBarriers() noexcept;
	private:
		
		[[nodiscard]] D3D12_RESOURCE_STATES GetLocalResourceState(const DeviceResource* pResource, uint32 subResource) const;
		
		void PrepareDraw() noexcept;
	private:

		struct PendingBarrier
		{
			DeviceResource* pResource = nullptr;
			D3D12_RESOURCE_STATES State{};
			uint32 Subresource = 0u;
		};

		Ref<ID3D12GraphicsCommandListX> m_pCommandList = nullptr;
		Ref<ID3D12CommandAllocator> m_pCommandAllocator = nullptr;
		CommandListContext m_CurrentCommandContext = CommandListContext::Invalid;
		D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

		std::vector<PendingBarrier> m_PendingBarriers;
		std::unordered_map<const DeviceResource*, ResourceState> m_ResourceStates;

		uint32_t m_NrOfBatchedBarriers = 0u;
		static constexpr uint32_t MAX_BATCHED_BARRIER_COUNT = 64u;
		std::array<D3D12_RESOURCE_BARRIER, MAX_BATCHED_BARRIER_COUNT> m_BatchedBarriers{};

		const PipelineState* m_pCurrentPSO = nullptr;
		const RootSignature* m_pCurrentComputeRS = nullptr;
		const RootSignature* m_pCurrentGraphicsRS = nullptr;

		RenderPassInfo m_CurrentRenderPassInfo;
		bool m_InRenderPass = false;

		ScratchAllocator m_ScratchAllocator;

		D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS m_ColorResolveParams;
		D3D12_RENDER_PASS_ENDING_ACCESS_RESOLVE_SUBRESOURCE_PARAMETERS m_DepthResolveParams;
	};

	namespace ComputeUtils
	{
		inline Vector3i GetNumThreadGroups(uint32 threadsX = 1, uint32 groupSizeX = 1, uint32 threadsY = 1, uint32 groupSizeY = 1, uint32 threadsZ = 1, uint32 groupSizeZ = 1)
		{
			Vector3i groups;
			groups.x = Math::DivideAndRoundUp(threadsX, groupSizeX);
			groups.y = Math::DivideAndRoundUp(threadsY, groupSizeY);
			groups.z = Math::DivideAndRoundUp(threadsZ, groupSizeZ);
			return groups;
		}

		inline Vector3i GetNumThreadGroups(const Vector3i& threads, const Vector3i& threadGroupSize)
		{
			Vector3i groups;
			groups.x = Math::DivideAndRoundUp(threads.x, threadGroupSize.x);
			groups.y = Math::DivideAndRoundUp(threads.y, threadGroupSize.y);
			groups.z = Math::DivideAndRoundUp(threads.z, threadGroupSize.z);
			return groups;
		}
	}
}