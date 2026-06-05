#pragma once
#include <Relentless.h>
#include "IStylableWidget.h"

namespace Relentless
{
	class RadioButton : public IStylableWidget<RadioButton>
	{
	public:
		RadioButton(StringView aText) noexcept;

		template<typename InstanceType>
		RadioButton* OnValueChanged(InstanceType* aInstance, void(InstanceType::*aMethod)(bool)) noexcept
		{
			m_OnValueChanged = [aInstance, aMethod](bool aValue) { return (aInstance->*aMethod)(aValue); };
			return this;
		}

		template<typename T>
		RadioButton* OnValueChanged(T&& aCallback) noexcept
		{
			m_OnValueChanged = Callback<void(bool aValue)>(std::forward<T>(aCallback));
			return this;
		}

		template<typename InstanceType>
		RadioButton* Value(InstanceType* aInstance, bool(InstanceType::*aMethod)() const) noexcept
		{
			m_ValueCallback = [aInstance, aMethod]() { return (aInstance->*aMethod)(); };
			return this;
		}

		template<typename T>
		RadioButton* Value(T&& aCallback) noexcept
		{
			m_ValueCallback = Callback<bool()>(std::forward<T>(aCallback));
			return this;
		}

	protected:
		void OnRender() noexcept override;
		NO_DISCARD Vector2 ReportSize() const noexcept override;
	private:
		String m_Text;
		Callback<bool()> m_ValueCallback;
		Callback<void(bool)> m_OnValueChanged;
	};
}