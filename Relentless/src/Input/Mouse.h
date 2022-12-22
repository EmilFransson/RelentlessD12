#pragma once
#include "../EventSystem/EventPublisher.h"
namespace Relentless
{
	enum class RLS_BUTTON : uint8_t {Left = 0u, Right, Wheel};
	constexpr const uint16_t BUTTON_COUNT{ 3 };
	class Mouse : public EventPublisher
	{
	public:
		static void OnButtonPressed(const RLS_BUTTON button) noexcept;
		static void OnButtonReleased(const RLS_BUTTON button) noexcept;
		static void OnMove(Vector2u newCoords) noexcept;
		static void OnRawDelta(Vector2i deltaCoords) noexcept;
		static void Reset() noexcept;
		static [[nodiscard]] const bool IsButtonPressed(const RLS_BUTTON button) noexcept;
		static [[nodiscard]] const std::pair<uint32_t, uint32_t> GetCoordinates() noexcept;
		static [[nodiscard]] const std::pair<int32_t, int32_t> GetDeltaCoordinates() noexcept;
	private:
		static std::bitset<BUTTON_COUNT> s_buttons;
		static Vector2u s_currentMouseCoords;
		static Vector2i s_deltaMouseCoords;
		STATIC_CLASS(Mouse);
	};
}