#pragma once
#include "IWidget.h"
#include "Callback/Callback.h"

namespace Relentless
{
	class CheckBox : public IStylableWidget<CheckBox>
	{
	public:
		CheckBox() noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		template<typename InstanceType>
		CheckBox* Value(InstanceType* instance, bool(InstanceType::* method)() const) noexcept
		{
			m_ValueCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		template<typename T>
		CheckBox* Value(T&& callback) noexcept
		{
			m_ValueCallback = Callback<bool()>(std::forward<T>(callback));
			return this;
		}

		template<typename T>
		CheckBox* OnCheckStateChanged(T&& callback) noexcept
		{
			m_OnCheckStateChanged = Callback<void(bool)>(std::forward<T>(callback));
			return this;
		}

	protected:
		virtual void OnRender() noexcept override;
	private:
		Callback<bool()> m_ValueCallback;
		Callback<void(bool isChecked)> m_OnCheckStateChanged;

		bool m_Hovered = false;
		bool m_MeasuredSize = false;
	};
}