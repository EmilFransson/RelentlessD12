#pragma once
#include "EventType.h"
#include "IEvent.h"
#include "MouseEvents.h"
#include "KeyboardEvents.h"

namespace Relentless
{
	class IEventListener
	{
	public:
		IEventListener() noexcept = default;
		virtual ~IEventListener() noexcept = default;
		virtual void OnEvent(IEvent& event) noexcept = 0;
	};
}