#include "Keyboard.h"
//#include "EventSystem/KeyboardEvents.h"
namespace Relentless
{
	std::bitset<KEY_COUNT> Keyboard::s_PersistentKeyStates{0};
	std::bitset<KEY_COUNT> Keyboard::s_CurrentKeyStates{0};
	bool Keyboard::m_sRepeatEnabled{ false };

	//void Keyboard::OnWindowsEvent(const uint32_t message, const LPARAM lParam, const WPARAM wParam) noexcept
	//{
	//	switch (message)
	//	{
	//	case WM_KEYDOWN:
	//	{
	//		BOOL repeatFlag = (HIWORD(lParam) & KF_REPEAT) == KF_REPEAT;
	//		if (repeatFlag == FALSE)
	//			OnKeyDown((RLS_KEY)wParam);
	//		else
	//			if (repeatFlag && IsRepeatEnabled())
	//				OnKeyDown((RLS_KEY)wParam);
	//		break;
	//	}
	//	case WM_SYSKEYDOWN:
	//	{
	//		//ALT+Enter handled from window class as of now.
	//		BOOL repeatFlag = (HIWORD(lParam) & KF_REPEAT) == KF_REPEAT;
	//		if (repeatFlag == FALSE)
	//			OnKeyDown((RLS_KEY)wParam);
	//		else
	//			if (repeatFlag && IsRepeatEnabled())
	//				OnKeyDown((RLS_KEY)wParam);
	//		break;
	//
	//		break;
	//	}
	//	case WM_KEYUP:
	//	{
	//		OnKeyUp((RLS_KEY)wParam);
	//		break;
	//	}
	//	}
	//}

	void Keyboard::UpdateKeyState(uint32 keyCode, bool isPressed) noexcept
	{
		s_PersistentKeyStates[(uint16)keyCode] = isPressed;
		s_CurrentKeyStates[(uint16)keyCode] = isPressed;
	}

	//void Keyboard::OnKeyDown(const RLS_KEY key) noexcept
	//{
	//	s_keys[(uint8_t)key] = true;
	//	PublishEvent<KeyPressedEvent>(key);
	//}
	//
	//void Keyboard::OnKeyUp(const RLS_KEY key) noexcept
	//{
	//	s_keys[(uint8_t)key] = false;
	//	PublishEvent<KeyReleasedEvent>(key);
	//}

	//void Keyboard::Reset() noexcept
	//{
	//	for (int i = 0; i < KEY_COUNT; i++)
	//	{
	//		if (s_keys[i])
	//		{
	//			OnKeyUp((RLS_KEY)i);
	//		}
	//	}
	//}

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