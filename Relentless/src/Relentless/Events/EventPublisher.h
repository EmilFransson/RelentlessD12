#pragma once
#include "WindowEvents.h"
#include "MouseEvents.h"
#include "KeyboardEvents.h"
class IEvent;

namespace Relentless
{
	class EventPublisher
	{
	public:
		EventPublisher() noexcept = default;
		virtual ~EventPublisher() noexcept = default;
		static void PublishEvent(IEvent&& event) noexcept;
	};
}