#pragma once
#include "..\Events\EventPublisher.h"
#include "MouseCodes.h"
namespace Relentless
{
	struct MouseCoordinates
	{
		uint32_t x;
		uint32_t y;
	};

	struct MouseDeltaCoordinates
	{
		int dx;
		int dy;
	};

	class Mouse : public EventPublisher
	{
	public:
		static const Mouse& Get() noexcept;
		[[nodiscard]] static bool IsButtonPressed(const RLS_MOUSE button) noexcept;
		static void OnButtonPressed(const RLS_MOUSE button) noexcept;
		static void OnButtonReleased(const RLS_MOUSE button) noexcept;
		static void OnMouseMove(const uint32_t xCoord, const uint32_t yCoord) noexcept;
		static void OnRawDelta(const int dx, const int dy) noexcept;
		static void OnWheelScrolled(const short wheelDelta) noexcept;
		static const std::pair<int, int> GetMovementDelta() noexcept;
		static constexpr MouseCoordinates& GetCursorCoordinates() noexcept { return m_sCurrentMouseCoords; }
		static MouseCoordinates m_sCurrentMouseCoords;
	private:
		static const Mouse s_Instance;
		static bool m_LeftButtonPressed;
		static bool m_RightButtonPressed;
		static bool m_WheelPressed;
		static MouseCoordinates m_sPreviousMouseCoords;
		static MouseDeltaCoordinates m_sDeltaCoords;
	};
}