#include "CommandQueue.h"
#include "CommandContext.h"
#include "Device.h"
#include "D3D.h"

namespace Relentless
{
	CommandQueue::CommandQueue(GraphicsDevice* pParent, D3D12_COMMAND_LIST_TYPE type) noexcept
		: DeviceObject{pParent}, m_Type{type}
	{
		const D3D12_COMMAND_QUEUE_DESC desc
		{
			.Type = type,
			.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL,
			.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
			.NodeMask = 0
		};

		VERIFY_HR_EX(GetParent()->GetDevice()->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pCommandQueue.ReleaseAndGetAddressOf())), GetParent()->GetDevice());
		VERIFY_HR_EX(m_pCommandQueue->GetTimestampFrequency(&m_TimeStampFrequency), GetParent()->GetDevice());
		
		D3D::SetObjectName(m_pCommandQueue, std::format("{} Command Queue", D3D::CommandListTypeToString(type)).c_str());
	
		m_pFence = new Fence(pParent, "Command Queue Fence");
	}

	ID3D12CommandQueue* CommandQueue::GetCommandQueue() const noexcept
	{
		return m_pCommandQueue;
	}

	D3D12_COMMAND_LIST_TYPE CommandQueue::GetType() const noexcept
	{
		return m_Type;
	}

	SyncPoint CommandQueue::ExecuteCommandLists(std::span<CommandContext* const> commandContexts) noexcept
	{
		RLS_ASSERT(!commandContexts.empty(), "[CommandQueue::ExecuteCommandLists] No Command Contexts Available For Execution.");

		std::vector<ID3D12CommandList*> commandLists;
		commandLists.reserve(commandContexts.size() + 1);

		CommandContext* pBarrierCommandlist = GetParent()->AllocateCommandContext(m_Type);
		CommandContext* pCurrentContext = pBarrierCommandlist;

		std::lock_guard guard(m_ExecutionMutex);

		for (CommandContext* pNextCommandContext : commandContexts)
		{
			RLS_ASSERT(pNextCommandContext, "[CommandQueue::ExecuteCommandLists] Command Context Is Invalid.");

			pNextCommandContext->ResolvePendingBarriers(*pCurrentContext);

			VERIFY_HR_EX(pCurrentContext->GetCommandList()->Close(), GetParent()->GetDevice());
			commandLists.push_back(pCurrentContext->GetCommandList());

			pCurrentContext = pNextCommandContext;
		}
		VERIFY_HR_EX(pCurrentContext->GetCommandList()->Close(), GetParent()->GetDevice());
		commandLists.push_back(pCurrentContext->GetCommandList());

		m_pCommandQueue->ExecuteCommandLists(static_cast<UINT>(commandLists.size()), commandLists.data());

		const uint64 fenceValue = m_pFence->Signal(this);
		m_SyncPoint = SyncPoint(m_pFence, fenceValue);

		pBarrierCommandlist->Free(m_SyncPoint);

		return m_SyncPoint;
	}

	void CommandQueue::FreeAllocator(const SyncPoint& syncPoint, Ref<ID3D12CommandAllocator> pAllocator) noexcept
	{
		std::lock_guard guard(m_AllocationMutex);
		m_AllocatorPool.push(FencedAllocator(std::move(pAllocator), syncPoint));
	}

	void CommandQueue::InsertWait(CommandQueue* pQueue)
	{
		InsertWait(pQueue->m_SyncPoint);
	}

	void CommandQueue::InsertWait(const SyncPoint& syncPoint)
	{
		if (syncPoint.IsValid())
			m_pCommandQueue->Wait(syncPoint.GetFence()->GetFence(), syncPoint.GetFenceValue());
	}

	Ref<ID3D12CommandAllocator> CommandQueue::RequestAllocator() noexcept
	{
		std::lock_guard guard(m_AllocationMutex);
		Ref<ID3D12CommandAllocator> pCommandAllocator = nullptr;
		if (!m_AllocatorPool.empty() && m_AllocatorPool.front().SyncPoint.IsComplete())
		{
			pCommandAllocator = std::move(m_AllocatorPool.front().pCommandAllocator);
			m_AllocatorPool.pop();
		}
		else
		{
			VERIFY_HR_EX(GetParent()->GetDevice()->CreateCommandAllocator(m_Type, IID_PPV_ARGS(pCommandAllocator.ReleaseAndGetAddressOf())), GetParent()->GetDevice());
			D3D::SetObjectName(pCommandAllocator, std::format("Pooled Allocator - {}", D3D::CommandListTypeToString(m_Type)).c_str());
		}

		VERIFY_HR_EX(pCommandAllocator->Reset(), GetParent()->GetDevice());
		return pCommandAllocator;
	}

	void CommandQueue::WaitForIdle() noexcept
	{
		const uint64 lastSignaled = m_pFence->Signal(this);
		m_pFence->CPUWait(lastSignaled);
	}

}

