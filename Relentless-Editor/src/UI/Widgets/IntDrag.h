#pragma once
#include <Relentless.h>
#include "IWidget.h"

namespace Relentless
{
	class IntDrag : public IStylableWidget<IntDrag>
	{
	public:
		IntDrag(float aSpeed = 1.0f, int aMin = INT_MIN , int aMax = INT_MAX, const char* aFormat = "", int someFlags = 0) noexcept;

		template<typename T>
		IntDrag* Value(T&& callback) noexcept;

		virtual void OnRender() noexcept override;

		template<typename T>
		IntDrag* OnValueChanged(T&& callback) noexcept;

		NO_DISCARD Vector2 ReportSize() const noexcept override;

		void SetDrawColorIndicator(bool state) noexcept;

		void SetHandleColor(const Color& color) noexcept;
		void SetHandleSize(float size) noexcept;

		IntDrag* SetIndicatorColor(const Color& color) noexcept;

		Broadcaster<void(bool state)> OnActiveChanged;
	private:
		void SetActive(bool state) noexcept;
		void OnMouseRawMove(const Vector2i& delta) noexcept;
	private:
		String m_Format;
		Callback<int32()> m_ValueCallback;
		Callback<void(int32 value)> m_OnChanged;

		Color m_IndicatorColor = Colors::White;
		Vector2i m_Delta = Vector2i::Zero();

		int32 m_Min = INT_MIN;
		int32 m_Max = INT_MAX;
		float m_Speed = 1;

		bool m_IsHovered = false;
		bool m_IsUsing = false;
		bool m_IsActive = false;
		bool m_DrawColorIndicator = false;
	};

	template<typename T>
	IntDrag* IntDrag::OnValueChanged(T&& callback) noexcept
	{
		m_OnChanged = Callback<void(int32)>(std::forward<T>(callback));
		return this;
	}

	template<typename T>
	IntDrag* IntDrag::Value(T&& callback) noexcept
	{
		m_ValueCallback = Callback<int32()>(std::forward<T>(callback));
		return this;
	}
}