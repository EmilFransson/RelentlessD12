#pragma once
namespace Relentless
{
	enum class EventCategory : uint8 { WindowEventCategory = 0u, MouseEventCategory, KeyboardEventCategory };
	enum class EventType : uint8 
	{ 
		WindowResizedEvent = 0u, WindowPosChangingEvent, WindowClosedEvent, WindowActiveEvent, WindowHitBorderEvent, WindowAltEnterEvent, WindowGainedFocusEvent, WindowLostFocusEvent,
		LeftMouseButtonPressedEvent, LeftMouseButtonReleasedEvent, RightMouseButtonPressedEvent, RightMouseButtonReleasedEvent, MiddleMouseButtonPressedEvent, MiddleMouseButtonReleasedEvent,
		MouseMovedEvent, RawMouseMoveEvent, MouseWheelScrolledEvent,
		KeyPressedEvent, KeyReleasedEvent
	};

	class IEvent
	{
	public:
		IEvent() noexcept : m_DonePropagating{false} {};
		virtual ~IEvent() noexcept = default;
		void StopPropagation() noexcept { m_DonePropagating = true; }
		NO_DISCARD bool IsValid() const { return !m_DonePropagating; }
		NO_DISCARD virtual EventType GetEventType() const = 0;
		NO_DISCARD virtual EventCategory GetEventCategory() const = 0;
	private:
		bool m_DonePropagating;
	};

#ifndef EVENT
	#define EVENT(derived) static_cast<derived&>(event)
#endif
}