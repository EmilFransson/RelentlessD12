#include "Keyboard.h"

namespace Relentless
{
	const Keyboard Keyboard::m_sInstance;
	std::bitset<KEY_COUNT> Keyboard::m_sKeyStates{ false };
	bool Keyboard::m_sRepeatEnabled{ false };

	const Keyboard& Keyboard::Get() noexcept
	{
		return m_sInstance;
	}

	void Keyboard::OnKeyDown(const WPARAM key) noexcept
	{
		m_sKeyStates[key] = true;
		PublishEvent(KeyDownEvent(static_cast<RLS_KEY>(key)));
	}

	void Keyboard::OnKeyRelease(const WPARAM key) noexcept
	{
		m_sKeyStates[key] = false;
		PublishEvent(KeyReleaseEvent(static_cast<RLS_KEY>(key)));
	}

	const bool Keyboard::IsKeyDown(const RLS_KEY key) noexcept
	{
		return m_sKeyStates[static_cast<unsigned int>(key)];
	}
}