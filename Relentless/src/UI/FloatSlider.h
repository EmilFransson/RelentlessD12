#pragma once
#include "IWidget.h"

namespace Relentless
{
	class FloatSlider : public IWidget
	{
	public:
		FloatSlider(std::string_view id, float startValue, float min, float max, const char* pFormat = "", int flags = 0) noexcept;

		[[nodiscard]] float GetValue() const noexcept;
		virtual void OnPreRender() noexcept override;
		virtual void OnRender() noexcept override;

		Broadcaster<void(float value)> OnChanged;
	private:
		void SetColorsAndStyles() noexcept;
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