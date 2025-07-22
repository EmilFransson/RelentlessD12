#include "Keyboard.h"
namespace Relentless
{
	std::bitset<KEY_COUNT> Keyboard::s_PersistentKeyStates{0};
	std::bitset<KEY_COUNT> Keyboard::s_CurrentKeyStates{0};
	bool Keyboard::m_sRepeatEnabled{ false };
	Broadcaster<void(RLS_Key, bool)> Keyboard::OnKeyStateChanged;

	void Keyboard::UpdateKeyState(uint32 keyCode, bool isPressed) noexcept
	{
		s_PersistentKeyStates[(uint16)keyCode] = isPressed;
		s_CurrentKeyStates[(uint16)keyCode] = isPressed;

		OnKeyStateChanged((RLS_Key)(uint16)keyCode, isPressed);
	}

	void Keyboard::Update() noexcept
	{
		s_CurrentKeyStates.reset();
	}

	bool Keyboard::IsKeyDown(const RLS_Key key) noexcept
	{
		return s_PersistentKeyStates[(uint16)key];
	}

	bool Keyboard::IsKeyPressed(const RLS_Key key) noexcept
	{
		return s_PersistentKeyStates[(uint16)key] && s_CurrentKeyStates[(uint16)key];
	}

}