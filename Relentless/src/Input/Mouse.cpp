#include "Mouse.h"
//#include "EventSystem/MouseEvents.h"
namespace Relentless
{
	std::bitset<(uint16)RLS_Button::Count> Mouse::s_PersistentStates;
	std::bitset<(uint16)RLS_Button::Count> Mouse::s_CurrentStates;
	Vector2u Mouse::s_CurrentMouseCoords;
	Vector2i Mouse::s_DeltaMouseCoords = Vector2i(0,0);
	float Mouse::s_MouseWheeel = 0.0f;
	bool Mouse::s_CursorVisible{ true };

	[[nodiscard]] RLS_Button Mouse::KeyCodeToButton(uint32 keyCode) noexcept
	{
		switch (keyCode)
		{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
			return RLS_Button::Left;
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
			return RLS_Button::Right;
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
			return RLS_Button::Wheel;
		}

		return RLS_Button::Unsupported;
	}

	//void Mouse::OnWindowsEvent(const uint32 message, const LPARAM lParam, [[maybe_unused]] const WPARAM wParam) noexcept
	//{
	//	switch (message)
	//	{
	//	case WM_MOUSEMOVE:
	//	{
	//		OnMove({ (uint32)GET_X_LPARAM(lParam), (uint32)GET_Y_LPARAM(lParam) });
	//
	//		break;
	//	}
	//	case WM_INPUT:
	//	{
	//		UINT size = 0u;
	//		if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
	//			break;
	//
	//		std::vector<char> rawBuffer;
	//		rawBuffer.resize(size);
	//		if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
	//			break;
	//
	//		auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
	//		if (ri.header.dwType == RIM_TYPEMOUSE && (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
	//		{
	//			OnRawDelta({ ri.data.mouse.lLastX, ri.data.mouse.lLastY });
	//		}
	//
	//		break;
	//	}
	//	case WM_LBUTTONDOWN:
	//	{
	//		OnButtonPressed(RLS_Button::Left);
	//		break;
	//	}
	//	case WM_LBUTTONUP:
	//	{
	//		OnButtonReleased(RLS_Button::Left);
	//		break;
	//	}
	//	case WM_RBUTTONDOWN:
	//	{
	//		OnButtonPressed(RLS_Button::Right);
	//		break;
	//	}
	//	case WM_RBUTTONUP:
	//	{
	//		OnButtonReleased(RLS_Button::Right);
	//		break;
	//	}
	//	case WM_MBUTTONDOWN:
	//	{
	//		OnButtonPressed(RLS_Button::Wheel);
	//		break;
	//	}
	//	case WM_MBUTTONUP:
	//	{
	//		OnButtonReleased(RLS_Button::Wheel);
	//		break;
	//	}
	//	}
	//}

	//void Mouse::OnButtonPressed(const RLS_Button button) noexcept
	//{
	//	s_PersistentStates[(uint8_t)button] = true;
	//	switch (button)
	//	{
	//	case RLS_Button::Left: PublishEvent<LeftMouseButtonPressedEvent>(s_CurrentMouseCoords); break;
	//	case RLS_Button::Right: PublishEvent<RightMouseButtonPressedEvent>(s_CurrentMouseCoords); break;
	//	case RLS_Button::Wheel: PublishEvent<MiddleMouseButtonPressedEvent>(s_CurrentMouseCoords); break;
	//	}
	//}
	//
	//void Mouse::OnButtonReleased(const RLS_Button button) noexcept
	//{
	//	s_PersistentStates[(uint8_t)button] = false;
	//	switch (button)
	//	{
	//	case RLS_Button::Left: PublishEvent<LeftMouseButtonReleasedEvent>(s_CurrentMouseCoords); break;
	//	case RLS_Button::Right: PublishEvent<RightMouseButtonReleasedEvent>(s_CurrentMouseCoords); break;
	//	case RLS_Button::Wheel: PublishEvent<MiddleMouseButtonReleasedEvent>(s_CurrentMouseCoords); break;
	//	}
	//}

	void Mouse::OnMove(const Vector2u& newCoords) noexcept
	{
		if (s_CursorVisible)
		{
			s_CurrentMouseCoords = newCoords;
			//PublishEvent<MouseMovedEvent>(s_CurrentMouseCoords);
		}
	}

	void Mouse::OnRawDelta(const Vector2i& deltaCoords) noexcept
	{
		s_DeltaMouseCoords.x += deltaCoords.x;
		s_DeltaMouseCoords.y += deltaCoords.y;
		//PublishEvent<RawMouseMoveEvent>(s_DeltaMouseCoords);
	}

	void Mouse::Update() noexcept
	{
		s_CurrentStates.reset();
		s_DeltaMouseCoords = { 0,0 };
		s_MouseWheeel = 0.0f;
	}

	void Mouse::UpdateState(uint32 keyCode, bool isPressed) noexcept
	{
		const RLS_Button button = KeyCodeToButton(keyCode);
		s_PersistentStates[(uint16)button] = isPressed;
		s_CurrentStates[(uint16)button] = isPressed;
	}

	void Mouse::UpdateMouseWheel(float scrollAmount) noexcept
	{
		s_MouseWheeel = scrollAmount;
	}

	void Mouse::ConfineCursor(const float left, const float right, const float bottom, const float top) noexcept
	{
		RECT rect = {};
		rect.left = static_cast<LONG>(left);
		rect.right = static_cast<LONG>(right);
		rect.bottom = static_cast<LONG>(bottom);
		rect.top = static_cast<LONG>(top);
		::ClipCursor(&rect);
	}

	void Mouse::FreeCursor() noexcept
	{
		::ClipCursor(nullptr);
	}

	void Mouse::ShowCursor() noexcept
	{
		while (::ShowCursor(true) < 0);
		s_CursorVisible = true;

		RECT rect = {};
		::GetWindowRect(::GetActiveWindow(), &rect);
		constexpr const float horizontalPadding = 8.0f;
		constexpr const float verticalPadding = 31.0f;
		s_CurrentMouseCoords.x += static_cast<uint32_t>(rect.left + horizontalPadding);
		s_CurrentMouseCoords.y += static_cast<uint32_t>(rect.top + verticalPadding);
		::SetCursorPos(s_CurrentMouseCoords.x, s_CurrentMouseCoords.y);
	}

	void Mouse::HideCursor() noexcept
	{
		while (::ShowCursor(false) >= 0);
		s_CursorVisible = false;
	}

	bool Mouse::IsButtonDown(const RLS_Button button) noexcept
	{
		return s_PersistentStates[(uint8)button];
	}

	bool Mouse::IsButtonPressed(const RLS_Button button) noexcept
	{
		return s_PersistentStates[(uint8)button] && s_CurrentStates[(uint8)button];
	}

	const Vector2u& Mouse::GetCoordinates() noexcept
	{
		return s_CurrentMouseCoords;
	};

	const Vector2i& Mouse::GetDeltaCoordinates() noexcept
	{
		return s_DeltaMouseCoords;
	}
}