#include "Mouse.h"
#include "../EventSystem/MouseEvents.h"
namespace Relentless
{
	std::bitset<BUTTON_COUNT> Mouse::s_buttons;
	Vector2u Mouse::s_currentMouseCoords;
	Vector2i Mouse::s_deltaMouseCoords;
	bool Mouse::s_CursorVisible{ true };

	void Mouse::OnWindowsEvent(const uint32_t message, const LPARAM lParam, [[maybe_unused]] const WPARAM wParam) noexcept
	{
		switch (message)
		{
		case WM_MOUSEMOVE:
		{
			OnMove({ (uint32_t)GET_X_LPARAM(lParam), (uint32_t)GET_Y_LPARAM(lParam) });

			break;
		}
		case WM_INPUT:
		{
			UINT size = 0u;
			if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)) == -1)
				break;

			std::vector<char> rawBuffer;
			rawBuffer.resize(size);
			if (::GetRawInputData(reinterpret_cast<HRAWINPUT>(lParam), RID_INPUT, rawBuffer.data(), &size, sizeof(RAWINPUTHEADER)) != size)
				break;

			auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
			if (ri.header.dwType == RIM_TYPEMOUSE && (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
			{
				OnRawDelta({ ri.data.mouse.lLastX, ri.data.mouse.lLastY });
			}

			break;
		}
		case WM_LBUTTONDOWN:
		{
			OnButtonPressed(RLS_BUTTON::Left);
			break;
		}
		case WM_LBUTTONUP:
		{
			OnButtonReleased(RLS_BUTTON::Left);
			break;
		}
		case WM_RBUTTONDOWN:
		{
			OnButtonPressed(RLS_BUTTON::Right);
			break;
		}
		case WM_RBUTTONUP:
		{
			OnButtonReleased(RLS_BUTTON::Right);
			break;
		}
		case WM_MBUTTONDOWN:
		{
			OnButtonPressed(RLS_BUTTON::Wheel);
			break;
		}
		case WM_MBUTTONUP:
		{
			OnButtonReleased(RLS_BUTTON::Wheel);
			break;
		}
		}
	}

	void Mouse::OnButtonPressed(const RLS_BUTTON button) noexcept
	{
		s_buttons[(uint8_t)button] = true;
		switch (button)
		{
		case RLS_BUTTON::Left: PublishEvent<LeftMouseButtonPressedEvent>(s_currentMouseCoords); break;
		case RLS_BUTTON::Right: PublishEvent<RightMouseButtonPressedEvent>(s_currentMouseCoords); break;
		case RLS_BUTTON::Wheel: PublishEvent<MiddleMouseButtonPressedEvent>(s_currentMouseCoords); break;
		}
	}

	void Mouse::OnButtonReleased(const RLS_BUTTON button) noexcept
	{
		s_buttons[(uint8_t)button] = false;
		switch (button)
		{
		case RLS_BUTTON::Left: PublishEvent<LeftMouseButtonReleasedEvent>(s_currentMouseCoords); break;
		case RLS_BUTTON::Right: PublishEvent<RightMouseButtonReleasedEvent>(s_currentMouseCoords); break;
		case RLS_BUTTON::Wheel: PublishEvent<MiddleMouseButtonReleasedEvent>(s_currentMouseCoords); break;
		}
	}

	void Mouse::OnMove(Vector2u newCoords) noexcept
	{
		if (s_CursorVisible)
		{
			s_currentMouseCoords = newCoords;
			PublishEvent<MouseMovedEvent>(s_currentMouseCoords);
		}
	}

	void Mouse::OnRawDelta(Vector2i deltaCoords) noexcept
	{
		s_deltaMouseCoords.x += deltaCoords.x;
		s_deltaMouseCoords.y += deltaCoords.y;
		PublishEvent<RawMouseMoveEvent>(s_deltaMouseCoords);
	}

	void Mouse::Reset() noexcept
	{
		s_deltaMouseCoords = { 0,0 };
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
		s_currentMouseCoords.x += (rect.left + horizontalPadding);
		s_currentMouseCoords.y += (rect.top + verticalPadding);
		::SetCursorPos(s_currentMouseCoords.x, s_currentMouseCoords.y);
	}

	void Mouse::HideCursor() noexcept
	{
		while (::ShowCursor(false) >= 0);
		s_CursorVisible = false;
	}

	const bool Mouse::IsButtonPressed(const RLS_BUTTON button) noexcept
	{
		return s_buttons[(uint8_t)button];
	}

	const std::pair<uint32_t, uint32_t> Mouse::GetCoordinates() noexcept
	{
		return { s_currentMouseCoords.x, s_currentMouseCoords.y };
	};

	const std::pair<int32_t, int32_t> Mouse::GetDeltaCoordinates() noexcept
	{
		return { s_deltaMouseCoords.x, s_deltaMouseCoords.y };
	}
}