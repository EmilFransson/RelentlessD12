#pragma once
#include "IWidget.h"

namespace Relentless
{
	class FloatSlider : public IStylableWidget
	{
	public:
		FloatSlider(std::string_view id, float startValue, float min, float max, const char* pFormat = "", int flags = 0) noexcept;

		virtual [[nodiscard]] float CalcDesiredWidth() const noexcept override;
		[[nodiscard]] float GetValue() const noexcept;

		void SetHandleColor(const Color& color) noexcept;
		void SetHandleSize(float size) noexcept;

		Broadcaster<void(float value)> OnChanged;
	protected:
		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;
	private:
		String m_Format;

		float m_Value = 0.0f;
		float m_Min = 0.0f;
		float m_Max = 0.0f;
		bool m_IsHovered = false;
		bool m_IsUsing = false;
		bool m_IsActive = false;
	};
}