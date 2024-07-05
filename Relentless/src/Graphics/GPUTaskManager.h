#pragma once
#include "Threading/ThreadSafeQueue.h"
#include "Callback/Callback.h"

namespace Relentless
{
	enum class CommandType : uint8_t { Direct = 0, Copy, Compute, Count };
	constexpr size_t COMMAND_TYPE_COUNT = static_cast<size_t>(CommandType::Count);

	class GPUTaskManager
	{
	public:
		static constexpr uint32_t FRAMES_IN_FLIGHT = 3;
		void Initialize() noexcept;
		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> RequestCommandList(CommandType commandType) noexcept;
		void ExecuteCommandListBlocking(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept;
		void ScheduleCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept;
		void ScheduleCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList, Callback<void()>&& callback) noexcept;
		void MoveToNextFrame() noexcept;
		[[nodiscard]] uint32_t GetCurrentFrameIndex() noexcept;
		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandQueue> GetCommandQueue(CommandType commandType) const noexcept;
		void WaitForAllFramesComplete() noexcept;
		void RequestBlockingCall(std::function<void()> func) noexcept;
		void Flush() noexcept;
		void EnsureFrameIsComplete(uint32_t frameIndex) noexcept;
		void SubmitOnFrameDoneCallback(Callback<void()>&& callback) noexcept;
		void InvokeAllCallbacks(uint32_t frameIndex) noexcept;
		void RecycleAllCommandListsForFrameIndex(uint32_t frameIndex);
	private:
		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> CreateCommandList(CommandType commandType, Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator) const noexcept;
		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(CommandType commandType) const noexcept;
		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12CommandQueue> CreateCommandQueue(CommandType commandType) const noexcept;
		[[nodiscard]] Microsoft::WRL::ComPtr<ID3D12Fence1> CreateFence() const noexcept;
		[[nodiscard]] D3D12_COMMAND_LIST_TYPE CommandTypeToD3D12CommandType(CommandType commandType) const noexcept;
		[[nodiscard]] CommandType D3D12CommandTypeToCommandType(D3D12_COMMAND_LIST_TYPE commandListType) const noexcept;
		void RecycleCommandList(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept;
		void ExecuteCommandListNonBlocking(Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> pCommandList) noexcept;
	private:
		std::atomic<uint32_t> m_CurrentFrameIndex = 0;
		std::array<Microsoft::WRL::ComPtr<ID3D12CommandQueue>, COMMAND_TYPE_COUNT> m_D3D12Queues;
		std::array<std::array<Microsoft::WRL::ComPtr<ID3D12Fence1>, FRAMES_IN_FLIGHT>, COMMAND_TYPE_COUNT> m_Fences;
		std::array<std::array<long long, FRAMES_IN_FLIGHT>, COMMAND_TYPE_COUNT> m_FenceValues;
		std::array<std::array<HANDLE, FRAMES_IN_FLIGHT>, COMMAND_TYPE_COUNT> m_Events;
		
		std::array<std::queue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>>, COMMAND_TYPE_COUNT> m_CommandLists;
		std::array<std::queue<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>, COMMAND_TYPE_COUNT> m_CommandAllocators;
		std::array<std::unordered_map<ID3D12GraphicsCommandList4*, Microsoft::WRL::ComPtr<ID3D12CommandAllocator>>, COMMAND_TYPE_COUNT> m_AllocatorsInUse;

		std::array<std::mutex, COMMAND_TYPE_COUNT> m_Mutexes;

		std::mutex m_ExecuteMutex;

		ThreadSafeQueue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>> m_RecordedCommandListsQueue;
		ThreadSafeQueue<Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>> m_CommandListsBeingExecuted[FRAMES_IN_FLIGHT];
		ThreadSafeQueue<Callback<void()>> m_FrameDoneCallbacks[FRAMES_IN_FLIGHT];
	};
}