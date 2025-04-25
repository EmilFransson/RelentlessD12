#pragma once
namespace Relentless
{
	enum class EventCategory : uint8_t { WindowEventCategory = 0u, MouseEventCategory, KeyboardEventCategory };
	enum class EventType : uint8_t 
	{ 
		WindowResizedEvent = 0u, WindowPosChangingEvent, WindowClosedEvent, WindowActiveEvent, WindowHitBorderEvent, WindowAltEnterEvent, WindowGainedFocusEvent, WindowLostFocusEvent,
		LeftMouseButtonPressedEvent, LeftMouseButtonReleasedEvent, RightMouseButtonPressedEvent, RightMouseButtonReleasedEvent, MiddleMouseButtonPressedEvent, MiddleMouseButtonReleasedEvent,
		MouseMovedEvent, RawMouseMoveEvent, MouseWheelScrolledEvent,
		KeyPressedEvent, KeyReleasedEvent
	};

	class IEvent
	{
	public:
		IEvent() noexcept : m_donePropagating{false} {};
		virtual ~IEvent() noexcept = default;
		void StopPropagation() noexcept { m_donePropagating = true; }
		[[nodiscard]] constexpr const bool IsValid() const { return !m_donePropagating; }
		[[nodiscard]] virtual constexpr const EventType GetEventType() const = 0;
		[[nodiscard]] virtual constexpr const EventCategory GetEventCategory() const = 0;
	private:
		bool m_donePropagating;
	};

#ifndef EVENT
	#define EVENT(derived) static_cast<derived&>(event)
#endif
}