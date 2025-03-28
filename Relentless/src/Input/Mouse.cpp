#include "Mouse.h"
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
		case VK_LBUTTON:
			return RLS_Button::Left;
		case VK_RBUTTON:
			return RLS_Button::Right;
		case VK_MBUTTON:
			return RLS_Button::Wheel;
		}

		return RLS_Button::Unsupported;
	}

	void Mouse::OnMove(const Vector2u& newCoords) noexcept
	{
		if (s_CursorVisible)
			s_CurrentMouseCoords = newCoords;
	}

	void Mouse::OnRawDelta(const Vector2i& deltaCoords) noexcept
	{
		s_DeltaMouseCoords.x += deltaCoords.x;
		s_DeltaMouseCoords.y += deltaCoords.y;
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
		s_CurrentMouseCoords.x += static_cast<uint32_t>(rect.left);
		s_CurrentMouseCoords.y += static_cast<uint32_t>(rect.top);
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

	const Vector2u& Mouse::GetCursorPosition() noexcept
	{
		return s_CurrentMouseCoords;
	};

	Vector2u Mouse::GetCursorScreenPosition() noexcept
	{
		POINT screenPosition{};
		::GetCursorPos(&screenPosition);
		return Vector2u(screenPosition.x, screenPosition.y);
	}

	const Vector2i& Mouse::GetDeltaCoordinates() noexcept
	{
		return s_DeltaMouseCoords;
	}
}