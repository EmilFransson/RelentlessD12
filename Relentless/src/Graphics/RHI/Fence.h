#pragma once
#include "DeviceResource.h"

namespace Relentless
{
	class Fence : public DeviceObject
	{
	public:
		Fence(GraphicsDevice* pParent, const char* pName, uint64_t fenceValue = 0u) noexcept;
		virtual ~Fence() noexcept override = default;

		void CPUWait(uint64_t fenceValue) noexcept;
		void CPUWait() noexcept;

		[[nodiscard]] uint64_t GetCurrentValue() const noexcept;
		[[nodiscard]] ID3D12Fence1* GetFence() const noexcept;
		[[nodiscard]] uint64_t GetLastSignaledValue() const noexcept;
		[[nodiscard]] bool IsComplete(uint64_t fenceValue) noexcept;
		uint64_t Signal(uint64_t fenceValue) noexcept;
		uint64_t Signal(CommandQueue* pCommandQueue) noexcept;

	private:
		Ref<ID3D12Fence1> m_pFence = nullptr;
		HANDLE m_EventHandle;
		uint64_t m_CurrentValue = 0u;
		uint64_t m_LastCompleted = 0u;
		uint64_t m_LastSignaled = 0u;

		std::mutex m_FenceMutex;
	};

	class SyncPoint
	{
	public:
		SyncPoint() = default;
		SyncPoint(Fence* pFence, uint64_t fenceValue) noexcept;
		~SyncPoint() noexcept = default;

		[[nodiscard]] Fence* GetFence() const noexcept;
		[[nodiscard]] uint64_t GetFenceValue() const noexcept;

		void Wait() noexcept;
		[[nodiscard]] bool IsComplete() const noexcept;
		[[nodiscard]] bool IsValid() const noexcept;
	private:
		Fence* m_pFence = nullptr;
		uint64_t m_FenceValue = 0u;
	};
}