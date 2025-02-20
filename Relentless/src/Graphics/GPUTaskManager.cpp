#include "GPUTaskManager.h"
#include "D3D12Core.h"
#include "Core/Application.h"
#include "Core/Window.h"
#include "Graphics/D3D12Debug.h"


namespace Relentless
{
	void GPUTaskManager::Initialize() noexcept
	{
		m_D3D12Queues[static_cast<size_t>(CommandType::Direct)] = CreateCommandQueue(CommandType::Direct);
		m_D3D12Queues[static_cast<size_t>(CommandType::Copy)] = CreateCommandQueue(CommandType::Copy);
		m_D3D12Queues[static_cast<size_t>(CommandType::Compute)] = CreateCommandQueue(CommandType::Compute);

		for (uint8_t i = 0u; i < COMMAND_TYPE_COUNT; ++i)
		{
			const CommandType commandType = static_cast<CommandType>(i);
			for (uint32_t j = 0; j < 500; ++j)
			{
				Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator = CreateCommandAllocator(commandType);
				Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = CreateCommandList(commandType, pCommandAllocator);
#if defined RLS_DEBUG || defined RLS_RELWITHDEBINFO
				const std::wstring commandTypeString = commandType == CommandType::Direct ? L"Direct" : commandType == CommandType::Copy ? L"Copy" : L"Compute";
				const std::wstring commandAllocatorName = L"Command Allocator (command type: " + commandTypeString  + L")";
				NAME_D12_OBJECT_INDEXED(pCommandAllocator, commandAllocatorName.c_str(), j);
				const std::wstring commandListName = L"Command List (command type: " + std::to_wstring((int)commandType) + L")";
				NAME_D12_OBJECT_INDEXED(pCommandList, commandListName.c_str(), j);
#endif
				m_CommandLists[i].push(std::move(pCommandList));
				m_CommandAllocators[i].push(std::move(pCommandAllocator));
			}
		}

		for (uint8_t i = 0u; i < COMMAND_TYPE_COUNT; ++i)
		{
			const CommandType commandType = static_cast<CommandType>(i);
			for (uint8_t j = 0u; j < FRAMES_IN_FLIGHT; ++j)
			{
				m_Fences[i][j] = CreateFence();
				m_FenceValues[i][j] = 1u;
				m_Events[i][j] = ::CreateEvent(nullptr, false, false, nullptr);
			}
		}
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> GPUTaskManager::RequestCommandList(CommandType commandType) noexcept
	{
		const size_t typeIndex = static_cast<size_t>(commandType);
		std::lock_guard<std::mutex> lock(m_Mutexes[typeIndex]);
		
		if (!m_CommandLists[typeIndex].empty())
		{
			std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>>& commandListQueue = m_CommandLists[typeIndex];
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandList = std::move(commandListQueue.front());
			commandListQueue.pop();

			std::queue<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>& commandAllocatorQueue = m_CommandAllocators[typeIndex];
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = std::move(commandAllocatorQueue.front());
			commandAllocatorQueue.pop();

			DXCall(commandList->Reset(commandAllocator.Get(), nullptr));
			m_AllocatorsInUse[typeIndex][commandList.Get()] = std::move(commandAllocator);
			return commandList;
		}
		else
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator = CreateCommandAllocator(commandType);
			const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> commandList = CreateCommandList(commandType, commandAllocator);
			
			DXCall(commandList->Reset(commandAllocator.Get(), nullptr));
			m_AllocatorsInUse[typeIndex][commandList.Get()] = std::move(commandAllocator);
			return commandList;
		}
	}

	void GPUTaskManager::ExecuteCommandListBlocking(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		const size_t typeIndex = static_cast<size_t>(D3D12CommandTypeToCommandType(pCommandList->GetType()));
		long long fenceValue = 0;
		uint32_t currentFrameIndex = 0u;
		{
			std::lock_guard<std::mutex> guard(m_ExecuteMutex);

			currentFrameIndex = m_CurrentFrameIndex.load(std::memory_order_acquire);
			ID3D12CommandList* pCommandLists[] = { pCommandList.Get() };

			auto& commandQueue = m_D3D12Queues[typeIndex];
			auto& fence = m_Fences[typeIndex][currentFrameIndex];
			auto& event = m_Events[typeIndex][currentFrameIndex];

			DXCall(pCommandList->Close());
			DXCall_STD(commandQueue->ExecuteCommandLists(1, pCommandLists));

			fenceValue = m_FenceValues[typeIndex][currentFrameIndex];
			
			DXCall(commandQueue->Signal(fence.Get(), fenceValue));
		}
		
		EnsureFrameIsComplete(currentFrameIndex);
		RecycleCommandList(pCommandList);
	}

	void GPUTaskManager::ScheduleCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		//std::lock_guard<std::mutex> guard(m_ExecuteMutex);
		m_RecordedCommandListsQueue.Push(std::move(pCommandList));
	}

	void GPUTaskManager::ScheduleCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList, Callback<void()>&& callback) noexcept
	{
		//std::lock_guard<std::mutex> guard(m_ExecuteMutex);

		m_RecordedCommandListsQueue.Push(std::move(pCommandList));
		SubmitOnFrameDoneCallback(std::move(callback));
	}

	void GPUTaskManager::MoveToNextFrame() noexcept
	{
		PROFILE_FUNC;
		//std::lock_guard<std::mutex> guard(m_ExecuteMutex);

		//const uint32_t currentFrameIndex = m_CurrentFrameIndex.load(std::memory_order_acquire);
		//for (uint32_t i{ 0u }; i < COMMAND_TYPE_COUNT; ++i)
		//{
		//	const uint64_t currentFenceValue = m_FenceValues[i][currentFrameIndex];
		//	DXCall(m_D3D12Queues[i]->Signal(m_Fences[i][currentFrameIndex].Get(), currentFenceValue));
		//}

		const uint32_t nextFrameIndex = Window::GetCurrentBackbufferIndex();

		EnsureFrameIsComplete(nextFrameIndex);
		RecycleAllCommandListsForFrameIndex(nextFrameIndex);
		InvokeAllCallbacks(nextFrameIndex);

		//for (uint32_t i{ 0u }; i < COMMAND_TYPE_COUNT; ++i)
		//{
		//	m_FenceValues[i][nextFrameIndex]++;
		//}

		m_CurrentFrameIndex.store(nextFrameIndex, std::memory_order_release);
	}

	uint32_t GPUTaskManager::GetCurrentFrameIndex() noexcept
	{
		return m_CurrentFrameIndex.load(std::memory_order_acquire);
	}

	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> GPUTaskManager::CreateCommandList(CommandType commandType, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator) const noexcept
	{
		const D3D12_COMMAND_LIST_TYPE d3dCommandListType = CommandTypeToD3D12CommandType(commandType);

		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pNewCommandList = nullptr;
		DXCall(D3D12Core::GetDevice()->CreateCommandList(0, d3dCommandListType, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pNewCommandList)));
		DXCall(pNewCommandList->Close());
		return pNewCommandList;
	}

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> GPUTaskManager::CreateCommandAllocator(CommandType commandType) const noexcept
	{
		const D3D12_COMMAND_LIST_TYPE d3dCommandListType = CommandTypeToD3D12CommandType(commandType);

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pNewCommandAllocator = nullptr;
		DXCall(D3D12Core::GetDevice()->CreateCommandAllocator(d3dCommandListType, IID_PPV_ARGS(&pNewCommandAllocator)));
		return pNewCommandAllocator;
	}

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GPUTaskManager::CreateCommandQueue(CommandType commandType) const noexcept
	{
		const D3D12_COMMAND_LIST_TYPE d3dCommandListType = CommandTypeToD3D12CommandType(commandType);

		D3D12_COMMAND_QUEUE_DESC commandQueueDescriptor;
		commandQueueDescriptor.Type = d3dCommandListType;
		commandQueueDescriptor.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		commandQueueDescriptor.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		commandQueueDescriptor.NodeMask = 0;

		Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue = nullptr;
		DXCall(D3D12Core::GetDevice()->CreateCommandQueue(&commandQueueDescriptor, IID_PPV_ARGS(&pCommandQueue)));

#if defined RLS_DEBUG || RLS_RELWITHDEBINFO
		const std::wstring name = L"Command Queue (type " + std::to_wstring((int)commandType) + L");";
		NAME_D12_OBJECT(pCommandQueue, name.c_str());
#endif

		return pCommandQueue;
	}

	Microsoft::WRL::ComPtr<ID3D12Fence1> GPUTaskManager::CreateFence() const noexcept
	{
		Microsoft::WRL::ComPtr<ID3D12Fence1> pFence = nullptr;
		DXCall(D3D12Core::GetDevice()->CreateFence(1, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));
		return pFence;
	}

	void GPUTaskManager::EnsureFrameIsComplete(uint32_t frameIndex) noexcept
	{
		std::lock_guard<std::mutex> guard(m_ExecuteMutex);
		for (size_t typeIndex = 0; typeIndex < COMMAND_TYPE_COUNT; ++typeIndex) 
		{
			DXCall(m_D3D12Queues[typeIndex]->Signal(m_Fences[typeIndex][frameIndex].Get(), ++m_FenceValues[typeIndex][frameIndex]));
			long long lastKnownFenceValue = m_FenceValues[typeIndex][frameIndex];

			if (m_Fences[typeIndex][frameIndex]->GetCompletedValue() < lastKnownFenceValue) 
			{
				DXCall(m_Fences[typeIndex][frameIndex]->SetEventOnCompletion(lastKnownFenceValue, m_Events[typeIndex][frameIndex]));
				::WaitForSingleObjectEx(m_Events[typeIndex][frameIndex], INFINITE, FALSE);
			}
			//m_FenceValues[typeIndex][frameIndex]++;
		}
	}

	void GPUTaskManager::SubmitOnFrameDoneCallback(Callback<void()>&& callback) noexcept
	{
		m_FrameDoneCallbacks[GetCurrentFrameIndex()].Push(std::move(callback));
	}

	void GPUTaskManager::InvokeAllCallbacks(uint32_t frameIndex) noexcept
	{
		while (!m_FrameDoneCallbacks[frameIndex].Empty())
		{
			Callback<void()> callback;
			if (m_FrameDoneCallbacks[frameIndex].TryPop(callback))
			{
				callback();
			}
		}
	}

	D3D12_COMMAND_LIST_TYPE GPUTaskManager::CommandTypeToD3D12CommandType(CommandType commandType) const noexcept
	{
		switch (commandType)
		{
		case CommandType::Direct:
			return D3D12_COMMAND_LIST_TYPE_DIRECT;
		case CommandType::Copy:
			return D3D12_COMMAND_LIST_TYPE_COPY;
		case CommandType::Compute:
			return D3D12_COMMAND_LIST_TYPE_COMPUTE;
		}

		RLS_ASSERT(false, "[GPUTaskManager]: Unknown command type encountered.");
		return {};
	}

	CommandType GPUTaskManager::D3D12CommandTypeToCommandType(D3D12_COMMAND_LIST_TYPE commandListType) const noexcept
	{
		switch (commandListType)
		{
		case D3D12_COMMAND_LIST_TYPE_DIRECT:
			return CommandType::Direct;
		case D3D12_COMMAND_LIST_TYPE_COPY:
			return CommandType::Copy;
		case D3D12_COMMAND_LIST_TYPE_COMPUTE:
			return CommandType::Compute;
		}

		RLS_ASSERT(false, "[GPUTaskManager]: Unknown D3D12 command list type encountered.");
		return {};
	}

	void GPUTaskManager::RecycleAllCommandListsForFrameIndex(uint32_t frameIndex)
	{
		while (!m_CommandListsBeingExecuted[frameIndex].Empty())
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = nullptr;
			if (m_CommandListsBeingExecuted[frameIndex].TryPop(pCommandList))
			{
				RecycleCommandList(std::move(pCommandList));
			}
		}
	}

	void GPUTaskManager::RecycleCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		const size_t typeIndex = static_cast<size_t>(D3D12CommandTypeToCommandType(pCommandList->GetType()));
		std::lock_guard<std::mutex> guard(m_Mutexes[typeIndex]);

		RLS_ASSERT(m_AllocatorsInUse[typeIndex].contains(pCommandList.Get()), "Allocator does not exist.");

		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator = std::move(m_AllocatorsInUse[typeIndex][pCommandList.Get()]);
		DXCall(pCommandAllocator->Reset());

		m_AllocatorsInUse[typeIndex].erase(pCommandList.Get());

		std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>>& commandListQueue = m_CommandLists[typeIndex];
		commandListQueue.push(std::move(pCommandList));
		m_CommandAllocators[typeIndex].push(pCommandAllocator);
	}

	void GPUTaskManager::ExecuteCommandListNonBlocking(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept
	{
		const size_t typeIndex = static_cast<size_t>(D3D12CommandTypeToCommandType(pCommandList->GetType()));
		long long fenceValue = 0;
		uint32_t currentFrameIndex = 0u;
		{
			currentFrameIndex = m_CurrentFrameIndex.load(std::memory_order_acquire);
			ID3D12CommandList* pCommandLists[] = { pCommandList.Get() };

			DXCall(pCommandList->Close());
			DXCall_STD(m_D3D12Queues[typeIndex]->ExecuteCommandLists(1, pCommandLists));
		}
	}

	Microsoft::WRL::ComPtr<ID3D12CommandQueue> GPUTaskManager::GetCommandQueue(CommandType commandType) const noexcept
	{
		const size_t typeIndex = static_cast<size_t>(commandType);
		return m_D3D12Queues[typeIndex];
	}

	void GPUTaskManager::WaitForAllFramesComplete() noexcept
	{
		Flush();

		for (size_t typeIndex = 0; typeIndex < COMMAND_TYPE_COUNT; ++typeIndex)
		{
			for (uint8_t frameIndex = 0; frameIndex < FRAMES_IN_FLIGHT; ++frameIndex)
			{
				DXCall(m_D3D12Queues[typeIndex]->Signal(m_Fences[typeIndex][frameIndex].Get(), ++m_FenceValues[typeIndex][frameIndex]));
				long long lastKnownFenceValue = m_FenceValues[typeIndex][frameIndex];

				if (m_Fences[typeIndex][frameIndex]->GetCompletedValue() < lastKnownFenceValue)
				{
					DXCall(m_Fences[typeIndex][frameIndex]->SetEventOnCompletion(lastKnownFenceValue, m_Events[typeIndex][frameIndex]));
					::WaitForSingleObjectEx(m_Events[typeIndex][frameIndex], INFINITE, FALSE);
				}
			}
		}
	}

	void GPUTaskManager::RequestBlockingCall(std::function<void()> func) noexcept
	{
		WaitForAllFramesComplete();
		func();
	}

	void GPUTaskManager::Flush() noexcept
	{
		std::lock_guard<std::mutex> guard(m_ExecuteMutex);
		const uint32_t frameIndex = GetCurrentFrameIndex();
		std::vector<ID3D12CommandList*> commandLists;
		while (!m_RecordedCommandListsQueue.Empty())
		{
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList = nullptr;
			if (m_RecordedCommandListsQueue.TryPop(pCommandList))
			{
				pCommandList->Close();
				m_CommandListsBeingExecuted[frameIndex].Push(pCommandList);
				commandLists.push_back(pCommandList.Get());
			}
		}

		if (!commandLists.empty())
			m_D3D12Queues[0]->ExecuteCommandLists(static_cast<UINT>(commandLists.size()), commandLists.data());
	}
}