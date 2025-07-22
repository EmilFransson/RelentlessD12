#pragma once
#include "IWidget.h"

#include "Callback/Callback.h"

namespace Relentless
{
	class ITableRow : public IStylableWidget<ITableRow>
	{
	public:
		ITableRow() noexcept = default;
		virtual ~ITableRow() noexcept = default;

		template<typename T>
		ITableRow* OnClicked(T&& callback) noexcept
		{
			m_OnClickedCallback = Callback<void()>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		ITableRow* OnClicked(InstanceType* instance, void(InstanceType::* method)()) noexcept
		{
			m_OnClickedCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		template<typename T>
		ITableRow* OnDoubleClicked(T&& callback) noexcept
		{
			m_OnDoubleClickedCallback = Callback<void()>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		ITableRow* OnDoubleClicked(InstanceType* instance, void(InstanceType::* method)()) noexcept
		{
			m_OnDoubleClickedCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		const Color& GetBackgroundColor() const noexcept 
		{
			return m_BackgroundColor;
		}

		ITableRow* SetBackgroundColor(const Color& backgroundColor) noexcept override
		{
			m_BackgroundColor = backgroundColor;
			return this;
		}
	protected:
		Callback<void()> m_OnClickedCallback;
		Callback<void()> m_OnDoubleClickedCallback;

		Color m_BackgroundColor = Colors::Transparent;
	};
}