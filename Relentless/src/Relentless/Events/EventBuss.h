#pragma once
#include "IEvent.h"
#include "EventType.h"
#include "IEventListener.h"
#include "../Application.h"

namespace Relentless
{
	class EventBuss
	{
	private:
		static EventBuss m_sInstance;
	private:
		EventBuss() noexcept = default;
		~EventBuss() noexcept = default;
	public:
		[[nodiscard]] static EventBuss& Get() noexcept;
		void Dispatch(IEvent& event) const;
		void SetMainApplication(Application* pApplication) noexcept;

		Application* m_pMainApplication;
	};
}