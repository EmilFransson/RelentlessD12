#pragma once

#include "Core/DLLExport.h"
#include "Callback/Broadcaster.h"

#include "Math/MathTypes.h"

namespace Relentless
{
	enum class RLS_Button : int {None = -1, Left = 0, Right, Wheel, Unsupported, Count};
	
	struct PointerInfo
	{
		RLS_Button EffectingButton = RLS_Button::None;
		std::unordered_set<RLS_Button> PressedButtons;
		float WheelDelta = 0.0f;
		Vector2u LocalPosition = Vector2u(0u, 0u);
		Vector2u ScreenSpacePosition = Vector2u(0u, 0u);
	};

	class RLS_API Mouse
	{
	public:
		NO_DISCARD static PointerInfo CreatePointerInfo() noexcept;
		static void OnMove(const Vector2u& newCoords) noexcept;
		static void OnRawDelta(const Vector2i& deltaCoords) noexcept;
		static void Update() noexcept;
		static void UpdateState(uint32 keyCode, bool isPressed) noexcept;
		static void UpdateMouseWheel(float scrollAmount) noexcept;
		static void ConfineCursor(const float left, const float right, const float bottom, const float top) noexcept;
		static void FreeCursor() noexcept;
		static void ShowCursor() noexcept;
		static void HideCursor() noexcept;
		NO_DISCARD static bool IsCursorConfined() noexcept;
		NO_DISCARD static bool IsButtonDown(const RLS_Button button) noexcept;
		NO_DISCARD static bool IsButtonPressed(const RLS_Button button) noexcept;
		NO_DISCARD static bool IsButtonReleased(const RLS_Button button) noexcept;
		NO_DISCARD static const Vector2u& GetCursorPosition() noexcept;
		NO_DISCARD static Vector2u GetCursorScreenPosition() noexcept;
		NO_DISCARD static const Vector2i& GetDeltaCoordinates() noexcept;
		NO_DISCARD static RLS_Button KeyCodeToButton(uint32 keyCode) noexcept;

		inline static Broadcaster<void(const Vector2i& delta)> OnRawMove;
	private:
		inline static std::bitset<(uint16)RLS_Button::Count> s_CurrentStates;
		inline static std::bitset<(uint16)RLS_Button::Count> s_PressedThisFrame;
		inline static std::bitset<(uint16)RLS_Button::Count> s_PreviousStates;
		inline static std::bitset<(uint16)RLS_Button::Count> s_ReleasedThisFrame;
		inline static Vector2u s_CurrentMouseCoords = Vector2u(0u, 0u);
		inline static Vector2i s_DeltaMouseCoords = Vector2i(0, 0);
		inline static float s_MouseWheeel = 0.0f;
		inline static bool s_CursorVisible = true;
		inline static bool s_Confined = false;

		STATIC_CLASS(Mouse);
	};
}