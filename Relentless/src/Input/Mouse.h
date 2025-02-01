#pragma once
#include "EventSystem/EventPublisher.h"
#include "Math/MathTypes.h"	
namespace Relentless
{
	enum class RLS_BUTTON : uint8 {Left = 0u, Right, Wheel};
	constexpr const uint16_t BUTTON_COUNT{ 3 };
	class Mouse : public EventPublisher
	{
	public:
		static void OnWindowsEvent(const uint32 message, const LPARAM lParam, const WPARAM wParam) noexcept;
		static void OnButtonPressed(const RLS_BUTTON button) noexcept;
		static void OnButtonReleased(const RLS_BUTTON button) noexcept;
		static void OnMove(Vector2u newCoords) noexcept;
		static void OnRawDelta(Vector2i deltaCoords) noexcept;
		static void Reset() noexcept;
		static void ConfineCursor(const float left, const float right, const float bottom, const float top) noexcept;
		static void FreeCursor() noexcept;
		static void ShowCursor() noexcept;
		static void HideCursor() noexcept;
		static [[nodiscard]] const bool IsButtonPressed(const RLS_BUTTON button) noexcept;
		static [[nodiscard]] const std::pair<uint32, uint32> GetCoordinates() noexcept;
		static [[nodiscard]] const Vector2i GetDeltaCoordinates() noexcept;
	private:
		static std::bitset<BUTTON_COUNT> s_buttons;
		static Vector2u s_currentMouseCoords;
		static Vector2i s_deltaMouseCoords;
		static bool s_CursorVisible;
		STATIC_CLASS(Mouse);
	};
}