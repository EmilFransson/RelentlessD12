#include "EventBuss.h"
#include "IEventListener.h"
#include "LayerStack.h"
#include "Layer.h"
#include "../Log.h"

namespace Relentless
{
	EventBuss EventBuss::m_sInstance;

	EventBuss& EventBuss::Get() noexcept
	{
		return m_sInstance;
	}

	void EventBuss::Dispatch(IEvent& event) const
	{
		for (auto it = LayerStack::Get().end(); it != LayerStack::Get().begin();)
		{
			if (event.IsHandled())
				return;

			(*--it)->OnEvent(event);
		}
		//If no layer handles the event the event might be targeted at the application itself
		m_pMainApplication->OnEvent(event);
	}

	void EventBuss::SetMainApplication(Application* pApplication) noexcept
	{
		RLS_ASSERT(pApplication != nullptr, "Application pointer was null");
		m_pMainApplication = pApplication;
	}
}
