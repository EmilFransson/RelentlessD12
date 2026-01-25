#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class FloatEntryBox : public IStylableWidget<FloatEntryBox>
	{
	public:
		FloatEntryBox(float aStep = 1.0f, const char* pFormat = "", int flags = 0) noexcept;

		NO_DISCARD virtual float CalcDesiredWidth() const noexcept override;

		template<typename T>
		FloatEntryBox* Value(T&& callback) noexcept;

		template<typename InstanceType>
		FloatEntryBox* Value(InstanceType* instance, float(InstanceType::*method)()) noexcept;

		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;

		template<typename T>
		FloatEntryBox* OnValueChanged(T&& callback) noexcept;

		template<typename InstanceType>
		FloatEntryBox* OnValueChanged(InstanceType* instance, void(InstanceType::*method)(float)) noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		void SetDrawColorIndicator(bool state) noexcept;
		void SetFormat(const char* aFormat) noexcept;
		void SetHandleColor(const Color& color) noexcept;
		void SetHandleSize(float size) noexcept;

		FloatEntryBox* SetIndicatorColor(const Color& color) noexcept;

		Broadcaster<void(bool state)> OnActiveChanged;
	private:
		void SetActive(bool state) noexcept;
	private:
		String m_Format;
		Callback<float()> m_ValueCallback;
		Callback<void(float aValue)> m_OnChanged;

		Color m_IndicatorColor = Colors::White;
		Vector2i m_Delta = Vector2i::Zero();

		float m_Step = 1.0f;
		bool m_IsActive = false;
		bool m_DrawColorIndicator = false;
	};

	template<typename T>
	FloatEntryBox* FloatEntryBox::OnValueChanged(T&& callback) noexcept
	{
		m_OnChanged = Callback<void(float)>(std::forward<T>(callback));
		return this;
	}

	template<typename InstanceType>
	FloatEntryBox* FloatEntryBox::OnValueChanged(InstanceType* instance, void(InstanceType::*method)(float)) noexcept
	{
		m_OnChanged = [instance, method](float aValue) { return (instance->*method)(aValue); };
		return this;
	}

	template<typename T>
	FloatEntryBox* FloatEntryBox::Value(T&& callback) noexcept
	{
		m_ValueCallback = Callback<float()>(std::forward<T>(callback));
		return this;
	}

	template<typename InstanceType>
	FloatEntryBox* FloatEntryBox::Value(InstanceType* instance, float(InstanceType::*method)()) noexcept
	{
		m_ValueCallback = [instance, method]() { return (instance->*method)(); };
		return this;
	}
}