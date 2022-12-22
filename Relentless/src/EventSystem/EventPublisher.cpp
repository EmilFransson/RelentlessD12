#include "EventPublisher.h"
namespace Relentless
{
	void EventPublisher::PublishEvent(IEvent&& event) noexcept
	{
		EventBus::Get().Dispatch(std::move(event));
	}
}