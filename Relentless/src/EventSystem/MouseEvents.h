#pragma once
#include "IEvent.h"
namespace Relentless
{
	class LeftMouseButtonPressedEvent : public IEvent
	{
	public:
		explicit LeftMouseButtonPressedEvent(const Vector2u& coordinates) noexcept : coordinates{ coordinates } {}
		virtual ~LeftMouseButtonPressedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::LeftMouseButtonPressedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	public:
		Vector2u coordinates;
	};

	class LeftMouseButtonReleasedEvent : public IEvent
	{
	public:
		explicit LeftMouseButtonReleasedEvent(const Vector2u& coordinates) noexcept : coordinates{ coordinates } {}
		virtual ~LeftMouseButtonReleasedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::LeftMouseButtonReleasedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	public:
		Vector2u coordinates;
	};

	class RightMouseButtonPressedEvent : public IEvent
	{
	public:
		explicit RightMouseButtonPressedEvent(const Vector2u& coordinates) noexcept : coordinates{ coordinates } {}
		virtual ~RightMouseButtonPressedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::RightMouseButtonPressedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	public:
		Vector2u coordinates;
	};

	class RightMouseButtonReleasedEvent : public IEvent
	{
	public:
		explicit RightMouseButtonReleasedEvent(const Vector2u& coordinates) noexcept : coordinates{ coordinates } {}
		virtual ~RightMouseButtonReleasedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::RightMouseButtonReleasedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	public:
		Vector2u coordinates;
	};

	class MiddleMouseButtonPressedEvent : public IEvent
	{
	public:
		explicit MiddleMouseButtonPressedEvent(const Vector2u& coordinates) noexcept : coordinates{ coordinates } {}
		virtual ~MiddleMouseButtonPressedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::MiddleMouseButtonPressedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	public:
		Vector2u coordinates;
	};

	class MiddleMouseButtonReleasedEvent : public IEvent
	{
	public:
		explicit MiddleMouseButtonReleasedEvent(const Vector2u& coordinates) noexcept : coordinates{ coordinates } {}
		virtual ~MiddleMouseButtonReleasedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::MiddleMouseButtonReleasedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	public:
		Vector2u coordinates;
	};

	class MouseMovedEvent : public IEvent
	{
	public:
		explicit MouseMovedEvent(const Vector2u& coordinates) noexcept : coordinates{ coordinates } {}
		virtual ~MouseMovedEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::MouseMovedEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	public:
		Vector2u coordinates;
	};

	class RawMouseMoveEvent : public IEvent
	{
	public:
		RawMouseMoveEvent(Vector2i deltaCoordinates) noexcept : deltaCoordinates{ deltaCoordinates } {}
		virtual ~RawMouseMoveEvent() noexcept override = default;
		
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::RawMouseMoveEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
		NO_DISCARD const Vector2i& GetDeltaCoordinates() const noexcept { return deltaCoordinates;}
	private:
		Vector2i deltaCoordinates;
	};

	class MouseWheelScrolledEvent : public IEvent
	{
	public:
		MouseWheelScrolledEvent(float delta) noexcept : Delta{ delta } {}
		virtual ~MouseWheelScrolledEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::MouseWheelScrolledEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override final { return EventCategory::MouseEventCategory; }
	public:
		float Delta = 0.0f;
	};
}
