#pragma once

namespace Relentless
{
	class Any
	{
	public:
		Any() = default;

		template <typename T>
		Any(T&& value) noexcept : m_Data(std::forward<T>(value)) {}

		Any(const Any& other) : m_Data(other.m_Data) {}
		Any(Any&& other) noexcept : m_Data(std::move(other.m_Data)) {}
		Any& operator=(const Any& other)
		{
			if (this != &other)
			{
				m_Data = other.m_Data;
			}
			return *this;
		}

		// Move assignment operator
		Any& operator=(Any&& other) noexcept
		{
			if (this != &other)
			{
				m_Data = std::move(other.m_Data);
			}
			return *this;
		}

		// Templated assignment operator to allow storing any type
		template <typename T>
		Any& operator=(T&& value) noexcept
		{
			m_Data = std::forward<T>(value);
			return *this;
		}

		template <typename T>
		[[nodiscard]] const T* Get() const noexcept
		{
			if (m_Data.type() == typeid(T))
			{
				try
				{
					return std::any_cast<T>(&m_Data);
				}
				catch (const std::bad_any_cast&)
				{
					return nullptr;
				}
			}
			return nullptr;
		}

		[[nodiscard]] bool HasValue() const noexcept
		{
			return m_Data.has_value();
		}

		void Reset() noexcept
		{
			m_Data.reset();
		}

	private:
		std::any m_Data;
	};
}
