#pragma once
#include "Callback/Broadcaster.h"

namespace Relentless
{
	template<typename TSignature>
	class ScopedConnection
	{
	public:
		ScopedConnection() noexcept = default;
		ScopedConnection(Broadcaster<TSignature>& aBroadcaster, auto&& aCallback)
			: m_pBroadcaster(&aBroadcaster), m_ID(aBroadcaster.Connect(std::forward<decltype(aCallback)>(aCallback))) {}
		~ScopedConnection() { Reset(); }

		ScopedConnection(ScopedConnection&& aOther) noexcept { *this = std::move(aOther); }
		ScopedConnection& operator=(ScopedConnection&& aOther) noexcept
		{
			Reset();
			m_pBroadcaster = std::exchange(aOther.m_pBroadcaster, nullptr);
			m_ID = aOther.m_ID;
			return *this;
		}
		ScopedConnection(const ScopedConnection&) = delete;
		ScopedConnection& operator=(const ScopedConnection&) = delete;

		void Reset() noexcept
		{
			if (m_pBroadcaster && m_pBroadcaster->IsConnected(m_ID))
				m_pBroadcaster->Detach(m_ID);
			m_pBroadcaster = nullptr;
		}

	private:
		Broadcaster<TSignature>* m_pBroadcaster = nullptr;
		CallbackID m_ID{};
	};
}