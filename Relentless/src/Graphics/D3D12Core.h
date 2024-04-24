#pragma once
#include "D3D12Command.h"
#include "Threading/ThreadSafeQueue.h"
namespace Relentless
{
	struct D3D12SingleCommand
	{
		explicit D3D12SingleCommand(D3D12_COMMAND_LIST_TYPE commandType, uint8_t nrOfBufferedFrames);
		D3D12SingleCommand() {};

		D3D12_COMMAND_LIST_TYPE m_CommandType = D3D12_COMMAND_LIST_TYPE_DIRECT;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4> m_pCommandList = nullptr;
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_pCommandAllocator = nullptr;
		uint32_t m_FrameUsed = std::numeric_limits<uint32_t>::max();
	};

	class D3D12Core
	{
	public:
		static void Initialize() noexcept;
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12Device9>& GetDevice() noexcept { return m_pDevice; }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12CommandQueue>& GetCommandQueue() noexcept { return m_DirectCommandInterface.GetCommandQueue(); }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& GetCommandAllocator(const uint32_t index) noexcept { return m_DirectCommandInterface.GetCommandAllocator(index); }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList4>& GetCommandList(const uint32_t index = (GetCurrentFrame() % GetNrOfBufferedFrames())) noexcept { return m_DirectCommandInterface.GetCommandList(index); }
		[[nodiscard]] static constexpr Microsoft::WRL::ComPtr<IDXGIFactory7>& GetFactory() noexcept { return m_pFactory; }
		[[nodiscard]] static constexpr uint8_t GetNrOfBufferedFrames() noexcept { return m_NrOfBufferedFrames; }
		[[nodiscard]] static constexpr uint32_t GetCurrentFrame() noexcept { return m_CurrentFrame; }
		[[nodiscard]] static constexpr bool IsInitialized() noexcept { return m_IsInitialized; }
		static void AdvanceToNextFrame() noexcept 
		{ 
			std::lock_guard<std::mutex> guard(m_CommandMutex);
			m_CurrentFrame++; 
		}

		static void SubmitCommandResource(D3D12SingleCommand command) noexcept
		{
			//Thread safe queue!
			m_UsedCommandResources.Push(std::move(command));
		}

		[[nodiscard]] static std::vector<D3D12SingleCommand> GetAllUsedCommands() noexcept 
		{
			std::vector<D3D12SingleCommand> commands;
			
			while (!m_UsedCommandResources.Empty())
			{
				D3D12SingleCommand singleCommand;
				if (m_UsedCommandResources.TryPop(singleCommand))
					commands.push_back(std::move(singleCommand));
			}

			return commands;
		}

		static D3D12SingleCommand GetCommandResource() noexcept
		{
			std::unique_lock<std::mutex> lock(m_CommandMutex);

			m_CommandCondition.wait(lock, []() { return m_AvailableCommandResources.size() > 0; });

			const uint8_t currentFrameIndex = m_CurrentFrame % GetNrOfBufferedFrames();
			D3D12SingleCommand command = std::move(m_AvailableCommandResources[currentFrameIndex].front());
			m_AvailableCommandResources[currentFrameIndex].pop_front();

			command.m_FrameUsed = currentFrameIndex;

			command.m_pCommandList->Reset(command.m_pCommandAllocator.Get(), nullptr);

			return command;
		}

		static void InsertFreshCommand(D3D12SingleCommand command) noexcept
		{
			std::lock_guard<std::mutex> guard(m_CommandMutex);
			m_AvailableCommandResources[command.m_FrameUsed].push_back(std::move(command));
			m_CommandCondition.notify_one();
		}

	private:
		D3D12Core() noexcept = default;
		~D3D12Core() noexcept = default;
		static void CreateDebugAndValidationLayer() noexcept;
	private:
		static Microsoft::WRL::ComPtr<ID3D12Device9> m_pDevice;
		static Microsoft::WRL::ComPtr<IDXGIFactory7> m_pFactory;
		static D3D12Command m_DirectCommandInterface;
		static uint8_t m_NrOfBufferedFrames;
		static uint32_t m_CurrentFrame;
		static bool m_IsInitialized;
#if defined(RLS_DEBUG)
		static Microsoft::WRL::ComPtr<ID3D12Debug5> m_pDebugController;
		static Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_pInfoQueue;
#endif

		static std::vector<std::deque<D3D12SingleCommand>> m_AvailableCommandResources;
		static ThreadSafeQueue<D3D12SingleCommand> m_UsedCommandResources;

		static std::mutex m_CommandMutex;
		static std::condition_variable m_CommandCondition;
	};
}