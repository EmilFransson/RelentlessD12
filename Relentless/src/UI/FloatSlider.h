#pragma once
#include "Callback/Callback.h"
#include "IWidget.h"

namespace Relentless
{
	class FloatSlider : public IStylableWidget<FloatSlider>
	{
	public:
		FloatSlider(float min, float max, const char* pFormat = "", int flags = 0) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		void SetFormat(const char* pFormat) noexcept;
		void SetHandleColor(const Color& color) noexcept;
		void SetHandleSize(float size) noexcept;

		void SetMinValue(float value) noexcept;
		void SetMaxValue(float value) noexcept;

		template<typename T>
		FloatSlider* OnValueChanged(T&& callback) noexcept 
		{
			m_OnChanged = Callback<void(float)>(std::forward<T>(callback));
			return this;
		}

		template<typename InstanceType>
		FloatSlider* OnValueChanged(InstanceType* instance, void(InstanceType::* method)(float)) noexcept
		{
			m_OnChanged = [instance, method](float value) { return (instance->*method)(value); };
			return this;
		}

		template<typename InstanceType>
		FloatSlider* Value(InstanceType* instance, float(InstanceType::* method)() const) noexcept
		{
			m_ValueCallback = [instance, method]() { return (instance->*method)(); };
			return this;
		}

		template<typename T>
		FloatSlider* Value(T&& callback) noexcept
		{
			m_ValueCallback = Callback<float()>(std::forward<T>(callback));
			return this;
		}

		Broadcaster<void(bool state)> OnActiveChanged;
	protected:
		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;
	private:
		void SetActive(bool state) noexcept;
	private:
		String m_Format;
		Callback<float()> m_ValueCallback;
		Callback<void(float value)> m_OnChanged;

		float m_Min = 0.0f;
		float m_Max = 0.0f;
		bool m_IsHovered = false;
		bool m_IsUsing = false;
		bool m_IsActive = false;
	};
}