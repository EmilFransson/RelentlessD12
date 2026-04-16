#pragma once
#include <Relentless.h>
#include "IStylableWidget.h"

namespace Relentless
{
	class FloatDrag : public IStylableWidget<FloatDrag>
	{
	public:
		FloatDrag(float speed = 1.0f, float min = -FLT_MAX, float max = FLT_MAX, const char* pFormat = "", int flags = 0) noexcept;

		template<typename T>
		FloatDrag* Value(T&& callback) noexcept;

		virtual void OnRender() noexcept override;

		template<typename T>
		FloatDrag* OnValueChanged(T&& callback) noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		void SetDrawColorIndicator(bool state) noexcept;
		void SetHandleColor(const Color& color) noexcept;
		void SetHandleSize(float size) noexcept;
		FloatDrag* SetIndicatorColor(const Color& color) noexcept;

		Broadcaster<void(bool state)> OnActiveChanged;
	private:
		void SetActive(bool state) noexcept;
		void OnMouseRawMove(const Vector2i& delta) noexcept;
	private:
		String m_Format;
		Callback<float()> m_ValueCallback;
		Callback<void(float value)> m_OnChanged;

		Color m_IndicatorColor = Colors::White;
		Vector2i m_Delta = Vector2i::Zero();

		float m_Min = 0.0f;
		float m_Max = 0.0f;
		float m_Speed = 1.0f;

		bool m_IsHovered = false;
		bool m_IsUsing = false;
		bool m_IsActive = false;
		bool m_DrawColorIndicator = false;
	};

	template<typename T>
	FloatDrag* FloatDrag::OnValueChanged(T&& callback) noexcept
	{
		m_OnChanged = Callback<void(float)>(std::forward<T>(callback));
		return this;
	}

	template<typename T>
	FloatDrag* FloatDrag::Value(T&& callback) noexcept
	{
		m_ValueCallback = Callback<float()>(std::forward<T>(callback));
		return this;
	}
}