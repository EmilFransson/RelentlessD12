#pragma once
#include "..\Events\EventPublisher.h"
namespace Relentless
{
#define KEY_COUNT 256
	class Keyboard : public EventPublisher
	{
	public:
		static const Keyboard& Get() noexcept;
		static void OnKeyDown(const WPARAM key) noexcept;
		static void OnKeyRelease(const WPARAM key) noexcept;
		static const bool IsKeyDown(const RLS_KEY key) noexcept;
		static void constexpr EnableRepeat() noexcept { m_sRepeatEnabled = true; }
		static void constexpr DisableRepeat() noexcept { m_sRepeatEnabled = false; }
		static constexpr bool IsRepeatEnabled() noexcept { return m_sRepeatEnabled; }
	private:
		static std::bitset<KEY_COUNT> m_sKeyStates;
		static const Keyboard m_sInstance;
		static bool m_sRepeatEnabled;
	};
}