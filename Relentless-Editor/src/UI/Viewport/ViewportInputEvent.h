#pragma once
#include <Relentless.h>

namespace Relentless
{
	enum class EViewportInputType : uint8
	{
		None = 0u,

		KeyPressed,
		KeyReleased,

		MouseButtonPressed,
		MouseButtonReleased,

		MouseDragBegin,
		MouseDrag,
		MouseDragEnd,

		MouseWheel,

		FocusLost
	};

	struct ViewportInputEvent
	{
		EViewportInputType Type = EViewportInputType::None;
		RLS_Key Key = RLS_Key::None;
		RLS_Button Button = RLS_Button::None;

		PointerInfo PointerInfo{};
		KeyboardModifierMask KeyboardModifiers = KeyboardModifierMask::None;

		Vector2i ClientCoordinates = Vector2i(-1, -1);
		Vector2i MouseDelta = Vector2i::Zero();
		float WheelDelta = 0.0f;

		NO_DISCARD bool IsInsideClientArea() const noexcept
		{
			return ClientCoordinates.x >= 0 && ClientCoordinates.y >= 0;
		}

		NO_DISCARD bool IsKeyEvent() const noexcept
		{
			return Type == EViewportInputType::KeyPressed || Type == EViewportInputType::KeyReleased;
		}

		NO_DISCARD bool IsMouseButtonEvent() const noexcept
		{
			return Type == EViewportInputType::MouseButtonPressed || Type == EViewportInputType::MouseButtonReleased;
		}
	};
}