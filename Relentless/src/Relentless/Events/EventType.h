#pragma once
namespace Relentless
{
	enum class EventType {
		WindowCloseEvent = 0, WindowGainedFocusEvent, WindowLostFocusEvent, WindowResizeEvent, MouseMoveEvent, RawMouseMoveEvent, LeftMouseButtonPressedEvent,
		LeftMouseButtonReleasedEvent, RightMouseButtonPressedEvent, RightMouseButtonReleasedEvent, MouseWheelPressedEvent,
		MouseWheelReleasedEvent, MouseWheelScrolledEvent, KeyDownEvent, KeyReleaseEvent
	};

}