#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class IntSlider : public IStylableWidget<IntSlider>
	{
	public:
		IntSlider(int min, int max, const char* pFormat = "", int flags = 0) noexcept;

		void SetFormat(const char* pFormat) noexcept;
		void SetHandleColor(const Color& color) noexcept;
		void SetHandleSize(float size) noexcept;

		void SetMinValue(float value) noexcept;
		void SetMaxValue(float value) noexcept;

		template<typename T>
		IntSlider* OnValueChanged(T&& callback) noexcept
		{
			m_OnChanged = Callback<void(int)>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		IntSlider* OnValueChanged(InstanceType* instance, void(InstanceType::* method)(int)) noexcept
		{
			m_OnChanged = [instance, method](int value) { return (instance->*method)(value); };
			return this;
		}

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		template<typename InstanceType>
		IntSlider* Value(InstanceType* instance, int(InstanceType::* method)() const) noexcept
		{
			m_ValueCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		template<typename T>
		IntSlider* Value(T&& callback) noexcept
		{
			m_ValueCallback = Callback<int()>(std::forward<T>(callback));
			return this;
		}

		Broadcaster<void(bool state)> OnActiveChanged;
	protected:
		virtual void OnRender() noexcept override;
	private:
		void SetActive(bool state) noexcept;
	private:
		String m_Format;
		Callback<int()> m_ValueCallback;
		Callback<void(int value)> m_OnChanged;

		int m_Min = 0;
		int m_Max = 0;
		bool m_IsHovered = false;
		bool m_IsUsing = false;
		bool m_IsActive = false;
	};
}