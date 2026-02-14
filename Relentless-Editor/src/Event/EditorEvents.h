#pragma once
#include <Relentless.h>

namespace Relentless
{
	class MouseBeginDragEvent : public IEvent
	{
	public:
		MouseBeginDragEvent() noexcept{}
		virtual ~MouseBeginDragEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::MouseBeginDragEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	};

	class MouseDragEvent : public IEvent
	{
	public:
		MouseDragEvent(Vector2i aDeltaCoordinates) noexcept : DeltaCoordinates{ aDeltaCoordinates } {}
		virtual ~MouseDragEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::MouseDragEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }

		Vector2i DeltaCoordinates = Vector2i::Zero();
		bool LeftButtonDown = false;
		bool RightButtonDown = false;
		bool WheelDown = false;
	};

	class MouseEndDragEvent : public IEvent
	{
	public:
		MouseEndDragEvent() noexcept{}
		virtual ~MouseEndDragEvent() noexcept override = default;
		NO_DISCARD virtual EventType GetEventType() const override { return EventType::MouseEndDragEvent; }
		NO_DISCARD virtual EventCategory GetEventCategory() const override { return EventCategory::MouseEventCategory; }
	};
}
