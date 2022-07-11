#pragma once
#include "EventType.h"
#include "EventCategory.h"

namespace Relentless
{
	class IEvent
	{
	public:
		IEvent() noexcept;
		virtual ~IEvent() noexcept = default;
		[[nodiscard]] constexpr bool IsHandled() const noexcept { return m_Handled; }
		void constexpr MarkAsHandled() noexcept { m_Handled = true; }
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept = 0;
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept = 0;
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept = 0;
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept = 0;

		friend std::ostream& operator <<(std::ostream& os, const IEvent& event)
		{
			os << event.GetDebugInfo();
			return os;
		}
	private:
		bool m_Handled;
	};
}