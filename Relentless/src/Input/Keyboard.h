#pragma once

#include "Core/DLLExport.h"
#include "Callback/Broadcaster.h"

namespace Relentless
{
	enum class RLS_Key : uint16
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
		RShift = 161, RCtrl
	};

	constexpr const uint16 KEY_COUNT{ 256 };
	class RLS_API Keyboard
	{
	public:
		static void UpdateKeyState(uint32 keyCode, bool isPressed) noexcept;
		static void Update() noexcept;
		NO_DISCARD static bool IsKeyDown(const RLS_Key key) noexcept;
		NO_DISCARD static bool IsKeyPressed(const RLS_Key key) noexcept;
		static void constexpr EnableRepeat() noexcept { m_sRepeatEnabled = true; }
		static void constexpr DisableRepeat() noexcept { m_sRepeatEnabled = false; }
		static constexpr bool IsRepeatEnabled() noexcept { return m_sRepeatEnabled; }

		static Broadcaster<void(RLS_Key, bool)> OnKeyStateChanged;
	private:
		STATIC_CLASS(Keyboard);
	private:
		static std::bitset<KEY_COUNT> s_PersistentKeyStates;
		static std::bitset<KEY_COUNT> s_CurrentKeyStates;
		static bool m_sRepeatEnabled;
	};
}