#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class CheckBox : public IStylableWidget<CheckBox>
	{
	public:
		CheckBox() noexcept;

		NO_DISCARD virtual float CalcDesiredWidth() const noexcept override;

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

		template<typename InstanceType>
		CheckBox* OnCheckStateChanged(InstanceType* instance, void(InstanceType::*method)(bool)) noexcept
		{
			m_OnCheckStateChanged = [instance, method](bool aState) { return (instance->*method)(aState); };
			return this;
		}

		template<typename T>
		CheckBox* OnCheckStateChanged(T&& callback) noexcept
		{
			m_OnCheckStateChanged = Callback<void(bool)>(std::forward<T>(callback));
			return this;
		}

		NO_DISCARD Vector2 ReportSize() const noexcept override;
	protected:
		virtual void OnRender() noexcept override;
	private:
		Callback<bool()> m_ValueCallback;
		Callback<void(bool isChecked)> m_OnCheckStateChanged;

		bool m_Hovered = false;
	};
}