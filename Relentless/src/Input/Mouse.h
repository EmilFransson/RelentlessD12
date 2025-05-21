#pragma once
#include "Callback/Broadcaster.h"
#include "Math/MathTypes.h"

namespace Relentless
{
	enum class RLS_Button : uint8 {Left = 0u, Right, Wheel, Unsupported, Count};
	class Mouse
	{
	public:
		static void OnMove(const Vector2u& newCoords) noexcept;
		static void OnRawDelta(const Vector2i& deltaCoords) noexcept;
		static void Update() noexcept;
		static void UpdateState(uint32 keyCode, bool isPressed) noexcept;
		static void UpdateMouseWheel(float scrollAmount) noexcept;
		static void ConfineCursor(const float left, const float right, const float bottom, const float top) noexcept;
		static void FreeCursor() noexcept;
		static void ShowCursor() noexcept;
		static void HideCursor() noexcept;
		static [[nodiscard]] bool IsCursorConfined() noexcept;
		static [[nodiscard]] bool IsButtonDown(const RLS_Button button) noexcept;
		static [[nodiscard]] bool IsButtonPressed(const RLS_Button button) noexcept;
		static [[nodiscard]] const Vector2u& GetCursorPosition() noexcept;
		static [[nodiscard]] Vector2u GetCursorScreenPosition() noexcept;
		static [[nodiscard]] const Vector2i& GetDeltaCoordinates() noexcept;
		static [[nodiscard]] RLS_Button KeyCodeToButton(uint32 keyCode) noexcept;

		static Broadcaster<void(const Vector2i& delta)> OnRawMove;
	private:
		static std::bitset<(uint16)RLS_Button::Count> s_CurrentStates;
		static std::bitset<(uint16)RLS_Button::Count> s_PersistentStates;
		static Vector2u s_CurrentMouseCoords;
		static Vector2i s_DeltaMouseCoords;
		static float s_MouseWheeel;
		static bool s_CursorVisible;
		static bool s_Confined;
		STATIC_CLASS(Mouse);
	};
}