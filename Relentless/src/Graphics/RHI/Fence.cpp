#include "Fence.h"

#include "CommandQueue.h"
#include "D3D.h"
#include "Device.h"

namespace Relentless
{
	Fence::Fence(GraphicsDevice* pParent, const char* pName, uint64 fenceValue) noexcept
		: 
		DeviceObject(pParent),
		m_CurrentValue(fenceValue + 1),
		m_LastCompleted(fenceValue),
		m_LastSignaled(0)
	{
		VERIFY_HR_EX(pParent->GetDevice()->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.GetAddressOf())), pParent->GetDevice());
		D3D::SetObjectName(m_pFence.Get(), pName);
		m_EventHandle = ::CreateEventExA(nullptr, "Fence Event", 0, EVENT_ALL_ACCESS);
	}

	void Fence::CPUWait() noexcept
	{
		CPUWait(m_LastSignaled);
	}

	void Fence::CPUWait(uint64 fenceValue) noexcept
	{
		if (IsComplete(fenceValue))
			return;

		std::lock_guard guard(m_FenceMutex);
		m_pFence->SetEventOnCompletion(fenceValue, m_EventHandle);
		::WaitForSingleObject(m_EventHandle, INFINITE);

		m_LastCompleted = m_pFence->GetCompletedValue();
	}

	uint64 Fence::GetCurrentValue() const noexcept
	{
		return m_CurrentValue;
	}

	ID3D12Fence1* Fence::GetFence() const noexcept
	{
		return m_pFence;
	}

	uint64 Fence::GetLastSignaledValue() const noexcept
	{
		return m_LastSignaled;
	}

	bool Fence::IsComplete(uint64 fenceValue) noexcept
	{
		if (fenceValue <= m_LastCompleted)
			return true;

		m_LastCompleted = std::max(m_LastCompleted, m_pFence->GetCompletedValue());
		return fenceValue <= m_LastCompleted;
	}

	uint64 Fence::Signal(uint64_t fenceValue) noexcept
	{
		m_LastSignaled = fenceValue;
		m_LastCompleted = fenceValue;
		m_CurrentValue++;
		return m_LastSignaled;
	}

	uint64 Fence::Signal(CommandQueue* pCommandQueue) noexcept
	{
		pCommandQueue->GetCommandQueue()->Signal(m_pFence, m_CurrentValue);
		m_LastSignaled = m_CurrentValue;
		++m_CurrentValue;
		return m_LastSignaled;
	}

	SyncPoint::SyncPoint(Fence* pFence, uint64 fenceValue) noexcept
		: 
		m_pFence(pFence),
		m_FenceValue(fenceValue)
	{
	}

	Fence* SyncPoint::GetFence() const noexcept
	{
		return m_pFence;
	}

	uint64 SyncPoint::GetFenceValue() const noexcept
	{
		return m_FenceValue;
	}

	void SyncPoint::Wait() noexcept
	{
		m_pFence->CPUWait(m_FenceValue);
	}

	bool SyncPoint::IsComplete() const noexcept
	{
		return m_pFence->IsComplete(m_FenceValue);
	}

	bool SyncPoint::IsValid() const noexcept
	{
		return !!m_pFence;
	}

}