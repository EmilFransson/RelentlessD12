#pragma once
#include "DeviceResource.h"

namespace Relentless
{
	class Fence : public DeviceObject
	{
	public:
		Fence(GraphicsDevice* pParent, const char* pName, uint64 fenceValue = 0u) noexcept;
		virtual ~Fence() noexcept override = default;

		void CPUWait(uint64 fenceValue) noexcept;
		void CPUWait() noexcept;

		NO_DISCARD uint64 GetCurrentValue() const noexcept;
		NO_DISCARD ID3D12Fence1* GetFence() const noexcept;
		NO_DISCARD uint64 GetLastSignaledValue() const noexcept;
		NO_DISCARD bool IsComplete(uint64 fenceValue) noexcept;
		uint64 Signal(uint64 fenceValue) noexcept;
		uint64 Signal(CommandQueue* pCommandQueue) noexcept;

	private:
		Ref<ID3D12Fence1> m_pFence = nullptr;
		HANDLE m_EventHandle;
		uint64 m_CurrentValue = 0u;
		uint64 m_LastCompleted = 0u;
		uint64 m_LastSignaled = 0u;

		std::mutex m_FenceMutex;
	};

	class SyncPoint
	{
	public:
		SyncPoint() = default;
		SyncPoint(Fence* pFence, uint64 fenceValue) noexcept;
		~SyncPoint() noexcept = default;

		NO_DISCARD Fence* GetFence() const noexcept;
		NO_DISCARD uint64 GetFenceValue() const noexcept;

		void Wait() noexcept;
		NO_DISCARD bool IsComplete() const noexcept;
		NO_DISCARD bool IsValid() const noexcept;
	private:
		Fence* m_pFence = nullptr;
		uint64 m_FenceValue = 0u;
	};

	struct RenderJobState : public RefCounted<RenderJobState>
	{
		SyncPoint Sync;
		std::mutex Mutex;
		std::condition_variable ConditionVariable;
		bool Submitted = false;
	};

	class RLS_API RenderJobHandle
	{
	public:
		RenderJobHandle(const Ref<RenderJobState>& aRenderJobState) noexcept;
		RenderJobHandle() noexcept = default;
		virtual ~RenderJobHandle() noexcept = default;

		NO_DISCARD bool IsComplete() const noexcept;
		NO_DISCARD bool IsSubmitted() const noexcept;

		void Wait() noexcept;
	private:
		Ref<RenderJobState> m_pRenderJobState = nullptr;
	};
}