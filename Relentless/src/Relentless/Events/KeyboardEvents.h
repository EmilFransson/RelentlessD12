#pragma once
#include "IEvent.h"
#include "..\Input\KeyCodes.h"

namespace Relentless
{
	class KeyDownEvent : public IEvent
	{
	public:
		KeyDownEvent(const RLS_KEY key) noexcept
			: m_Key{ key }
		{}
		virtual ~KeyDownEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::KeyDownEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::KeyboardCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("KeyDownEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
#pragma region "RLS_KEY:s as strings"
			std::map<RLS_KEY, std::string> map = {};
			map[RLS_KEY::BACKSPACE] = "BACKSPACE";
			map[RLS_KEY::TAB] = "TAB";
			map[RLS_KEY::ENTER] = "ENTER";
			map[RLS_KEY::SHIFT] = "SHIFT";
			map[RLS_KEY::CTRL] = "CTRL";
			map[RLS_KEY::ALT] = "ALT";
			map[RLS_KEY::ESC] = "ESC";
			map[RLS_KEY::SPACE] = "SPACE";
			map[RLS_KEY::LEFT] = "LEFT";
			map[RLS_KEY::UP] = "UP";
			map[RLS_KEY::RIGHT] = "RIGHT";
			map[RLS_KEY::ZERO] = "0";
			map[RLS_KEY::ONE] = "1";
			map[RLS_KEY::TWO] = "2";
			map[RLS_KEY::THREE] = "3";
			map[RLS_KEY::FOUR] = "4";
			map[RLS_KEY::FIVE] = "5";
			map[RLS_KEY::SIX] = "6";
			map[RLS_KEY::SEVEN] = "7";
			map[RLS_KEY::EIGHT] = "8";
			map[RLS_KEY::NINE] = "9";
			map[RLS_KEY::A] = "A";
			map[RLS_KEY::B] = "B";
			map[RLS_KEY::C] = "C";
			map[RLS_KEY::D] = "D";
			map[RLS_KEY::E] = "E";
			map[RLS_KEY::F] = "F";
			map[RLS_KEY::G] = "G";
			map[RLS_KEY::H] = "H";
			map[RLS_KEY::I] = "I";
			map[RLS_KEY::J] = "J";
			map[RLS_KEY::K] = "K";
			map[RLS_KEY::L] = "L";
			map[RLS_KEY::M] = "M";
			map[RLS_KEY::N] = "N";
			map[RLS_KEY::O] = "O";
			map[RLS_KEY::P] = "P";
			map[RLS_KEY::Q] = "Q";
			map[RLS_KEY::R] = "R";
			map[RLS_KEY::S] = "S";
			map[RLS_KEY::T] = "T";
			map[RLS_KEY::U] = "U";
			map[RLS_KEY::V] = "V";
			map[RLS_KEY::W] = "W";
			map[RLS_KEY::X] = "X";
			map[RLS_KEY::Y] = "Y";
			map[RLS_KEY::Z] = "Z";
			map[RLS_KEY::F1] = "F1";
			map[RLS_KEY::F2] = "F2";
			map[RLS_KEY::F3] = "F3";
			map[RLS_KEY::F4] = "F4";
			map[RLS_KEY::F5] = "F5";
			map[RLS_KEY::F6] = "F6";
			map[RLS_KEY::F7] = "F7";
			map[RLS_KEY::F8] = "F8";
			map[RLS_KEY::F9] = "F9";
			map[RLS_KEY::F10] = "F10";
			map[RLS_KEY::F11] = "F11";
			map[RLS_KEY::F12] = "F12";

#pragma endregion
			std::stringstream ss = {};
			ss << GetDebugName() << ": " << map[m_Key];
			return ss.str();
		}
		[[nodiscard]] constexpr RLS_KEY GetKeyCode() const noexcept
		{
			return m_Key;
		}
	private:
		RLS_KEY m_Key;
	};

	class KeyReleaseEvent : public IEvent
	{
	public:
		KeyReleaseEvent(const RLS_KEY key) noexcept
			: m_Key{ key }
		{}
		virtual ~KeyReleaseEvent() noexcept override final = default;
		[[nodiscard]] virtual constexpr EventType GetEventType() const noexcept override final
		{
			return EventType::KeyReleaseEvent;
		}
		[[nodiscard]] virtual constexpr EventCategory GetEventCategory() const noexcept override final
		{
			return EventCategory::KeyboardCategory;
		}
		[[nodiscard]] virtual constexpr std::string GetDebugName() const noexcept override final
		{
			return std::string("KeyReleaseEvent");
		}
		[[nodiscard]] virtual const std::string GetDebugInfo() const noexcept override final
		{
#pragma region "RLS_KEY:s as strings"
			std::map<RLS_KEY, std::string> map = {};
			map[RLS_KEY::BACKSPACE] = "BACKSPACE";
			map[RLS_KEY::TAB] = "TAB";
			map[RLS_KEY::ENTER] = "ENTER";
			map[RLS_KEY::SHIFT] = "SHIFT";
			map[RLS_KEY::CTRL] = "CTRL";
			map[RLS_KEY::ALT] = "ALT";
			map[RLS_KEY::ESC] = "ESC";
			map[RLS_KEY::SPACE] = "SPACE";
			map[RLS_KEY::LEFT] = "LEFT";
			map[RLS_KEY::UP] = "UP";
			map[RLS_KEY::RIGHT] = "RIGHT";
			map[RLS_KEY::ZERO] = "0";
			map[RLS_KEY::ONE] = "1";
			map[RLS_KEY::TWO] = "2";
			map[RLS_KEY::THREE] = "3";
			map[RLS_KEY::FOUR] = "4";
			map[RLS_KEY::FIVE] = "5";
			map[RLS_KEY::SIX] = "6";
			map[RLS_KEY::SEVEN] = "7";
			map[RLS_KEY::EIGHT] = "8";
			map[RLS_KEY::NINE] = "9";
			map[RLS_KEY::A] = "A";
			map[RLS_KEY::B] = "B";
			map[RLS_KEY::C] = "C";
			map[RLS_KEY::D] = "D";
			map[RLS_KEY::E] = "E";
			map[RLS_KEY::F] = "F";
			map[RLS_KEY::G] = "G";
			map[RLS_KEY::H] = "H";
			map[RLS_KEY::I] = "I";
			map[RLS_KEY::J] = "J";
			map[RLS_KEY::K] = "K";
			map[RLS_KEY::L] = "L";
			map[RLS_KEY::M] = "M";
			map[RLS_KEY::N] = "N";
			map[RLS_KEY::O] = "O";
			map[RLS_KEY::P] = "P";
			map[RLS_KEY::Q] = "Q";
			map[RLS_KEY::R] = "R";
			map[RLS_KEY::S] = "S";
			map[RLS_KEY::T] = "T";
			map[RLS_KEY::U] = "U";
			map[RLS_KEY::V] = "V";
			map[RLS_KEY::W] = "W";
			map[RLS_KEY::X] = "X";
			map[RLS_KEY::Y] = "Y";
			map[RLS_KEY::Z] = "Z";
			map[RLS_KEY::F1] = "F1";
			map[RLS_KEY::F2] = "F2";
			map[RLS_KEY::F3] = "F3";
			map[RLS_KEY::F4] = "F4";
			map[RLS_KEY::F5] = "F5";
			map[RLS_KEY::F6] = "F6";
			map[RLS_KEY::F7] = "F7";
			map[RLS_KEY::F8] = "F8";
			map[RLS_KEY::F9] = "F9";
			map[RLS_KEY::F10] = "F10";
			map[RLS_KEY::F11] = "F11";
			map[RLS_KEY::F12] = "F12";
#pragma endregion
			std::stringstream ss = {};
			ss << GetDebugName() << ": " << map[m_Key];
			return ss.str();
		}
		[[nodiscard]] constexpr RLS_KEY GetKeyCode() const noexcept
		{
			return m_Key;
		}
	private:
		RLS_KEY m_Key;
	};
}