#pragma once
#include "Core/DLLExport.h"
#include "DeviceResource.h"
#include "Fence.h"

namespace Relentless
{
	class RLS_API CommandQueue : public DeviceObject
	{
	public:
		CommandQueue(GraphicsDevice* pParent, D3D12_COMMAND_LIST_TYPE type) noexcept;
		virtual ~CommandQueue() noexcept override = default;

		[[nodiscard]] ID3D12CommandQueue* GetCommandQueue() const noexcept;
		[[nodiscard]] D3D12_COMMAND_LIST_TYPE GetType() const noexcept;
		[[nodiscard]] SyncPoint ExecuteCommandLists(std::span<CommandContext* const> commandContexts) noexcept;
		void FreeAllocator(const SyncPoint& syncPoint, Ref<ID3D12CommandAllocator> pAllocator) noexcept;
		void InsertWait(const SyncPoint& syncPoint);
		void InsertWait(CommandQueue* pQueue);
		[[nodiscard]] Ref<ID3D12CommandAllocator> RequestAllocator() noexcept;
		void WaitForIdle() noexcept;
	private:
		struct FencedAllocator
		{
			FencedAllocator(Ref<ID3D12CommandAllocator> pInCommandAllocator, const SyncPoint& syncPoint) noexcept
				: pCommandAllocator{ pInCommandAllocator }, SyncPoint(syncPoint) 
			{}

			Ref<ID3D12CommandAllocator> pCommandAllocator = nullptr;
			SyncPoint SyncPoint;
		};
		std::queue<FencedAllocator> m_AllocatorPool;
		Ref<ID3D12CommandQueue> m_pCommandQueue = nullptr;
		D3D12_COMMAND_LIST_TYPE m_Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		Ref<Fence> m_pFence = nullptr;
		SyncPoint m_SyncPoint;
		std::mutex m_ExecutionMutex;
		std::mutex m_AllocationMutex;
		uint64 m_TimeStampFrequency = 0u;
	};
}