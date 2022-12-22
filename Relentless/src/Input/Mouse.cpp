#include "Mouse.h"
#include "../EventSystem/MouseEvents.h"
namespace Relentless
{
	std::bitset<BUTTON_COUNT> Mouse::s_buttons;
	Vector2u Mouse::s_currentMouseCoords;
	Vector2i Mouse::s_deltaMouseCoords;

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
		s_currentMouseCoords = newCoords;
		PublishEvent<MouseMovedEvent>(s_currentMouseCoords);
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