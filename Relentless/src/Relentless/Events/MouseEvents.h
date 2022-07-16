#pragma once
#include "IEvent.h"
namespace Relentless
{
	class MouseMoveEvent : public IEvent
	{
	public:
		MouseMoveEvent(const uint32_t xCoordinate, const uint32_t yCoordinate) noexcept
			: m_XCoordinate{ xCoordinate }, m_YCoordinate{ yCoordinate }
		{}
		virtual ~MouseMoveEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::MouseMoveEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("MouseMoveEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (x,y) = (" << std::to_string(m_XCoordinate) << "," << std::to_string(m_YCoordinate) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr uint32_t GetXCoordinate() const noexcept
		{
			return m_XCoordinate;
		}
		[[nodiscard]] constexpr uint32_t GetYCoordinate() const noexcept
		{
			return m_YCoordinate;
		}
	private:
		uint32_t m_XCoordinate;
		uint32_t m_YCoordinate;
	};

	class RawMouseMoveEvent : public IEvent
	{
	public:
		RawMouseMoveEvent(const int dx, const int dy) noexcept
			: m_DX{ dx }, m_DY{ dy }
		{}
		virtual ~RawMouseMoveEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::RawMouseMoveEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("RawMouseMoveEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (dx,dy) = (" << std::to_string(m_DX) << "," << std::to_string(m_DY) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr int GetXCoordinate() const noexcept
		{
			return m_DX;
		}
		[[nodiscard]] constexpr int GetYCoordinate() const noexcept
		{
			return m_DY;
		}
	private:
		int m_DX;
		int m_DY;
	};

	class LeftMouseButtonPressedEvent : public IEvent
	{
	public:
		LeftMouseButtonPressedEvent(const uint32_t xCoordinate, const uint32_t yCoordinate) noexcept
			: m_XCoordinate{ xCoordinate }, m_YCoordinate{ yCoordinate }
		{}
		virtual ~LeftMouseButtonPressedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::LeftMouseButtonPressedEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("LeftMouseButtonPressedEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (x,y) = (" << std::to_string(m_XCoordinate) << "," << std::to_string(m_YCoordinate) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr uint32_t GetXCoordinate() const noexcept
		{
			return m_XCoordinate;
		}
		[[nodiscard]] constexpr uint32_t GetYCoordinate() const noexcept
		{
			return m_YCoordinate;
		}
	private:
		uint32_t m_XCoordinate;
		uint32_t m_YCoordinate;
	};

	class LeftMouseButtonReleasedEvent : public IEvent
	{
	public:
		LeftMouseButtonReleasedEvent(const uint32_t xCoordinate, const uint32_t yCoordinate) noexcept
			: m_XCoordinate{ xCoordinate }, m_YCoordinate{ yCoordinate }
		{}
		virtual ~LeftMouseButtonReleasedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::LeftMouseButtonReleasedEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("LeftMouseButtonReleasedEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (x,y) = (" << std::to_string(m_XCoordinate) << "," << std::to_string(m_YCoordinate) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr uint32_t GetXCoordinate() const noexcept
		{
			return m_XCoordinate;
		}
		[[nodiscard]] constexpr uint32_t GetYCoordinate() const noexcept
		{
			return m_YCoordinate;
		}
	private:
		uint32_t m_XCoordinate;
		uint32_t m_YCoordinate;
	};

	class RightMouseButtonPressedEvent : public IEvent
	{
	public:
		RightMouseButtonPressedEvent(const uint32_t xCoordinate, const uint32_t yCoordinate) noexcept
			: m_XCoordinate{ xCoordinate }, m_YCoordinate{ yCoordinate }
		{}
		virtual ~RightMouseButtonPressedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::RightMouseButtonPressedEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("RightMouseButtonPressedEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (x,y) = (" << std::to_string(m_XCoordinate) << "," << std::to_string(m_YCoordinate) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr uint32_t GetXCoordinate() const noexcept
		{
			return m_XCoordinate;
		}
		[[nodiscard]] constexpr uint32_t GetYCoordinate() const noexcept
		{
			return m_YCoordinate;
		}
	private:
		uint32_t m_XCoordinate;
		uint32_t m_YCoordinate;
	};

	class RightMouseButtonReleasedEvent : public IEvent
	{
	public:
		RightMouseButtonReleasedEvent(const uint32_t xCoordinate, const uint32_t yCoordinate) noexcept
			: m_XCoordinate{ xCoordinate }, m_YCoordinate{ yCoordinate }
		{}
		virtual ~RightMouseButtonReleasedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::RightMouseButtonReleasedEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("RightMouseButtonReleasedEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (x,y) = (" << std::to_string(m_XCoordinate) << "," << std::to_string(m_YCoordinate) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr uint32_t GetXCoordinate() const noexcept
		{
			return m_XCoordinate;
		}
		[[nodiscard]] constexpr uint32_t GetYCoordinate() const noexcept
		{
			return m_YCoordinate;
		}
	private:
		uint32_t m_XCoordinate;
		uint32_t m_YCoordinate;
	};

	class MouseWheelPressedEvent : public IEvent
	{
	public:
		MouseWheelPressedEvent(const uint32_t xCoordinate, const uint32_t yCoordinate) noexcept
			: m_XCoordinate{ xCoordinate }, m_YCoordinate{ yCoordinate }
		{}
		virtual ~MouseWheelPressedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::MouseWheelPressedEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("MouseWheelPressedEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (x,y) = (" << std::to_string(m_XCoordinate) << "," << std::to_string(m_YCoordinate) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr uint32_t GetXCoordinate() const noexcept
		{
			return m_XCoordinate;
		}
		[[nodiscard]] constexpr uint32_t GetYCoordinate() const noexcept
		{
			return m_YCoordinate;
		}
	private:
		uint32_t m_XCoordinate;
		uint32_t m_YCoordinate;
	};

	class MouseWheelReleasedEvent : public IEvent
	{
	public:
		MouseWheelReleasedEvent(const uint32_t xCoordinate, const uint32_t yCoordinate) noexcept
			: m_XCoordinate{ xCoordinate }, m_YCoordinate{ yCoordinate }
		{}
		virtual ~MouseWheelReleasedEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::MouseWheelReleasedEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("MouseWheelReleasedEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": (x,y) = (" << std::to_string(m_XCoordinate) << "," << std::to_string(m_YCoordinate) << ")";
			return ss.str();
		}
		[[nodiscard]] constexpr uint32_t GetXCoordinate() const noexcept
		{
			return m_XCoordinate;
		}
		[[nodiscard]] constexpr uint32_t GetYCoordinate() const noexcept
		{
			return m_YCoordinate;
		}
	private:
		uint32_t m_XCoordinate;
		uint32_t m_YCoordinate;
	};

	enum class RLS_WHEEL { Up = 0, Down };
	class MouseWheelScrolledEvent : public IEvent
	{
	public:
		MouseWheelScrolledEvent(const RLS_WHEEL direction) noexcept
			: m_Direction{ direction }
		{}
		virtual ~MouseWheelScrolledEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::MouseWheelScrolledEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::MouseEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("MouseWheelScrolledEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName() << ": ";
			if (m_Direction == RLS_WHEEL::Up)
			{
				ss << "Up";
			}
			else
			{
				ss << "Down";
			}
			return ss.str();
		}
		[[nodiscard]] constexpr RLS_WHEEL GetScrollDirection() const noexcept
		{
			return m_Direction;
		}
	private:
		RLS_WHEEL m_Direction;
	};
}