#pragma once
#include "IEvent.h"

namespace Relentless
{
	class WindowCloseEvent : public IEvent
	{
	public:
		WindowCloseEvent() noexcept = default;
		virtual ~WindowCloseEvent() noexcept override = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override
		{
			return EventType::WindowCloseEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override
		{
			return EventCategory::WindowEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override
		{
			return std::string("WindowCloseEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override
		{
			std::stringstream ss;
			ss << GetDebugName();
			return ss.str();
		}
	};

	class WindowGainedFocusEvent : public IEvent
	{
	public:
		WindowGainedFocusEvent() noexcept = default;
		virtual ~WindowGainedFocusEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::WindowGainedFocusEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::WindowEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("WindowGainedFocusEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName();
			return ss.str();
		}
	};

	class WindowLostFocusEvent : public IEvent
	{
	public:
		WindowLostFocusEvent() noexcept = default;
		virtual ~WindowLostFocusEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::WindowLostFocusEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::WindowEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("WindowLostFocusEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName();
			return ss.str();
		}
	};

	class WindowResizeEvent : public IEvent
	{
	public:
		WindowResizeEvent(const uint32_t newWidth, const uint32_t newHeight) noexcept
			: m_Width{newWidth}, m_Height{newHeight}
		{}
		virtual ~WindowResizeEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::WindowResizeEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::WindowEventCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("WindowResizeEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
			std::stringstream ss;
			ss << GetDebugName();
			return ss.str();
		}
		[[nodiscard]] constexpr std::pair<uint32_t, uint32_t> GetNewDimension() const noexcept { return std::make_pair(m_Width, m_Height); }
	private:
		uint32_t m_Width;
		uint32_t m_Height;
	};
}