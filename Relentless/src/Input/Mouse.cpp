#include "Mouse.h"

namespace Relentless
{
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

	PointerInfo Mouse::CreatePointerInfo() noexcept
	{
		PointerInfo pointerInfo{};

		if (IsButtonReleased(RLS_Button::Left) || IsButtonPressed(RLS_Button::Left))
			pointerInfo.EffectingButton = RLS_Button::Left;
		else if (IsButtonReleased(RLS_Button::Right) || IsButtonPressed(RLS_Button::Right))
			pointerInfo.EffectingButton = RLS_Button::Right;
		else if (IsButtonReleased(RLS_Button::Wheel) || IsButtonPressed(RLS_Button::Wheel))
			pointerInfo.EffectingButton = RLS_Button::Wheel;
		else
			pointerInfo.EffectingButton = RLS_Button::None;

		if (IsButtonDown(RLS_Button::Left))
			pointerInfo.PressedButtons.insert(RLS_Button::Left);
		if (IsButtonDown(RLS_Button::Right))
			pointerInfo.PressedButtons.insert(RLS_Button::Right);
		if (IsButtonDown(RLS_Button::Wheel))
			pointerInfo.PressedButtons.insert(RLS_Button::Wheel);

		pointerInfo.WheelDelta = s_MouseWheeel;
		pointerInfo.LocalPosition = GetCursorPosition();
		pointerInfo.ScreenSpacePosition = GetCursorScreenPosition();

		return pointerInfo;
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

		OnRawMove(s_DeltaMouseCoords);
	}

	void Mouse::Update() noexcept
	{
		s_PreviousStates = s_CurrentStates;
		s_PressedThisFrame.reset();
		s_ReleasedThisFrame.reset();

		s_DeltaMouseCoords = { 0,0 };
		s_MouseWheeel = 0.0f;
	}

	void Mouse::UpdateState(uint32 keyCode, bool isPressed) noexcept
	{
		const RLS_Button button = KeyCodeToButton(keyCode);
	
		const bool wasDown = s_CurrentStates[(uint16)button];
		s_CurrentStates[(uint16)button] = isPressed;

		if (!wasDown && isPressed)
			s_PressedThisFrame.set((uint16)button);
		else if (wasDown && !isPressed)
			s_ReleasedThisFrame.set((uint16)button);
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
		s_Confined = true;
	}

	void Mouse::FreeCursor() noexcept
	{
		::ClipCursor(nullptr);
		s_Confined = false;
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

	bool Mouse::IsCursorConfined() noexcept
	{
		return s_Confined;
	}

	bool Mouse::IsButtonDown(const RLS_Button button) noexcept
	{
		return s_CurrentStates[(uint8)button];
	}

	bool Mouse::IsButtonPressed(const RLS_Button button) noexcept
	{
		return s_PressedThisFrame[(uint8)button];
	}

	bool Mouse::IsButtonReleased(const RLS_Button button) noexcept
	{
		return s_ReleasedThisFrame[(uint8)button];
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