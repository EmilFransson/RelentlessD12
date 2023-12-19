#pragma once
#include "EventSystem/EventPublisher.h"
namespace Relentless
{
	enum class RLS_KEY : uint8_t 
	{
		BackSpace = 8, Tab,
		Enter = 13,
		LShift = 16,
		LCtrl,
		Alt,
		Capslock = 20,
		Esc = 27,
		Spacebar = 32,
		LeftArrow = 37, UpArrow, RightArrow, DownArrow,
		Delete = 46, Zero = 48, One, Two, Three, Four, Five, Six, Seven, Eight, Nine,
		A = 65, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		F1 = 112, F2, F3,
		RShift = 161, RCtrl,
		Ö = 192,
		Å = 221, Ä
	};

	constexpr const uint16_t KEY_COUNT{ 256 };
	class Keyboard : public EventPublisher
	{
	public:
		static void OnWindowsEvent(const uint32_t message, const LPARAM lParam, const WPARAM wParam) noexcept;
		static void OnKeyDown(const RLS_KEY key) noexcept;
		static void OnKeyUp(const RLS_KEY key) noexcept;
		static void Reset() noexcept;
		static [[nodiscard]] const bool IsKeyPressed(const RLS_KEY key) noexcept;
		static void constexpr EnableRepeat() noexcept { m_sRepeatEnabled = true; }
		static void constexpr DisableRepeat() noexcept { m_sRepeatEnabled = false; }
		static constexpr bool IsRepeatEnabled() noexcept { return m_sRepeatEnabled; }
	private:
		STATIC_CLASS(Keyboard);
	private:
		static std::bitset<KEY_COUNT> s_keys;
		static bool m_sRepeatEnabled;
	};
}