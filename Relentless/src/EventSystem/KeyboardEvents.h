#pragma once
#include "IEvent.h"
#include "Input/Keyboard.h"

namespace Relentless
{
	class KeyPressedEvent : public IEvent
	{
	public:
		explicit KeyPressedEvent(RLS_Key key) noexcept : key{key}{}
		virtual ~KeyPressedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::KeyPressedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::KeyboardEventCategory; }
	public:
		RLS_Key key;
	};

	class KeyReleasedEvent : public IEvent
	{
	public:
		explicit KeyReleasedEvent(RLS_Key key) noexcept : key{ key } {}
		virtual ~KeyReleasedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::KeyReleasedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::KeyboardEventCategory; }
	public:
		RLS_Key key;
	};
}
