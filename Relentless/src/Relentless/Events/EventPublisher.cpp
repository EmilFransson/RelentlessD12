#include "EventPublisher.h"
#include "EventBuss.h"

namespace Relentless
{
	void EventPublisher::PublishEvent(IEvent&& event) noexcept
	{
		EventBuss::Get().Dispatch(event);
	}
}