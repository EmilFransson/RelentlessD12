#include "rlspch.h"
#include "Mouse.h"

namespace Relentless
{
	const Mouse Mouse::s_Instance;
	bool Mouse::m_LeftButtonPressed{ false };
	bool Mouse::m_RightButtonPressed{ false };
	bool Mouse::m_WheelPressed{ false };
	MouseCoordinates Mouse::m_sCurrentMouseCoords{ 0u, 0u };
	MouseCoordinates Mouse::m_sPreviousMouseCoords{ 0u, 0u };
	MouseDeltaCoordinates Mouse::m_sDeltaCoords{ 0u, 0u };

	const Mouse& Mouse::Get() noexcept
	{
		return s_Instance;
	}

	bool Mouse::IsButtonPressed(const RLS_MOUSE button) noexcept
	{
		switch (button)
		{
		case RLS_MOUSE::Left:
		{
			return m_LeftButtonPressed;
			break;
		}
		case RLS_MOUSE::Right:
		{
			return m_RightButtonPressed;
			break;
		}
		case RLS_MOUSE::Wheel:
		{
			return m_WheelPressed;
			break;
		}
		default:
		{
			//RLS_ASSERT(false, "Undefined mouse value.");
			return false;
			break;
		}
		}
	}

	void Mouse::OnButtonPressed(const RLS_MOUSE button) noexcept
	{
		switch (button)
		{
		case RLS_MOUSE::Left:
		{
			m_LeftButtonPressed = true;
			PublishEvent(LeftMouseButtonPressedEvent(m_sCurrentMouseCoords.x, m_sCurrentMouseCoords.y));
			break;
		}
		case RLS_MOUSE::Right:
		{
			m_RightButtonPressed = true;
			PublishEvent(RightMouseButtonPressedEvent(m_sCurrentMouseCoords.x, m_sCurrentMouseCoords.y));
			break;
		}
		case RLS_MOUSE::Wheel:
		{
			m_WheelPressed = true;
			PublishEvent(MouseWheelPressedEvent(m_sCurrentMouseCoords.x, m_sCurrentMouseCoords.y));
			break;
		}
		}
	}

	void Mouse::OnButtonReleased(const RLS_MOUSE button) noexcept
	{
		switch (button)
		{
		case RLS_MOUSE::Left:
		{
			m_LeftButtonPressed = false;
			PublishEvent(LeftMouseButtonReleasedEvent(m_sCurrentMouseCoords.x, m_sCurrentMouseCoords.y));
			break;
		}
		case RLS_MOUSE::Right:
		{
			m_RightButtonPressed = false;
			PublishEvent(RightMouseButtonReleasedEvent(m_sCurrentMouseCoords.x, m_sCurrentMouseCoords.y));
			break;
		}
		case RLS_MOUSE::Wheel:
		{
			m_WheelPressed = false;
			PublishEvent(MouseWheelReleasedEvent(m_sCurrentMouseCoords.x, m_sCurrentMouseCoords.y));
			break;
		}
		}
	}

	void Mouse::OnMouseMove(const uint32_t xCoord, const uint32_t yCoord) noexcept
	{
		m_sPreviousMouseCoords = m_sCurrentMouseCoords;
		m_sCurrentMouseCoords.x = xCoord;
		m_sCurrentMouseCoords.y = yCoord;
		PublishEvent(MouseMoveEvent(m_sCurrentMouseCoords.x, m_sCurrentMouseCoords.y));
	}

	void Mouse::OnRawDelta(const int dx, const int dy) noexcept
	{
		m_sDeltaCoords.dx = dx;
		m_sDeltaCoords.dy = dy;
		PublishEvent(RawMouseMoveEvent(dx, dy));
	}

	void Mouse::OnWheelScrolled(const short wheelDelta) noexcept
	{
		if (wheelDelta > 0)
		{
			PublishEvent(MouseWheelScrolledEvent(RLS_WHEEL::Up));
		}
		else
		{
			PublishEvent(MouseWheelScrolledEvent(RLS_WHEEL::Down));
		}
	}

	const std::pair<int, int> Mouse::GetMovementDelta() noexcept
	{
		return std::make_pair(m_sDeltaCoords.dx, m_sDeltaCoords.dy);
	}
}