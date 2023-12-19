#pragma once
#include "IEvent.h"
#include "Input/Keyboard.h"

namespace Relentless
{
	class KeyPressedEvent : public IEvent
	{
	public:
		explicit KeyPressedEvent(RLS_KEY key) noexcept : key{key}{}
		virtual ~KeyPressedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr const EventType GetEventType() const { return EventType::KeyPressedEvent; }
		[[nodiscard]] virtual constexpr const EventCategory GetEventCategory() const { return EventCategory::KeyboardEventCategory; }
	public:
		RLS_KEY key;
	};

	class KeyReleasedEvent : public IEvent
	{
	public:
		explicit KeyReleasedEvent(RLS_KEY key) noexcept : key{ key } {}
		virtual ~KeyReleasedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr const EventType GetEventType() const { return EventType::KeyReleasedEvent; }
		[[nodiscard]] virtual constexpr const EventCategory GetEventCategory() const { return EventCategory::KeyboardEventCategory; }
	public:
		RLS_KEY key;
	};
}
