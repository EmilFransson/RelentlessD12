#include "FloatSlider.h"
#include "Input/Mouse.h"

namespace Relentless
{
	FloatSlider::FloatSlider(float min, float max, const char* pFormat, int flags) noexcept
		:m_Format{pFormat}
		,m_Min{ min }
		,m_Max{ max }
	{
		if (pFormat == nullptr || pFormat[0] == '\0')
			m_Format = "%.2f";
		else
			m_Format = pFormat;

		SetFlags(flags);

		const Color defaultFrameColor = Colors::Normalize(12.5f, 12.5f, 12.5f, 255.0f);
		SetBackgroundColor(defaultFrameColor);
		SetHoverColor(defaultFrameColor);
		SetActiveColor(defaultFrameColor);
		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetHandleColor(Colors::Normalize(90.0f, 90.0f, 90.0f, 255.0f));

		SetHandleSize(20.0f);
		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);
		SetPadding(Vector2(6.0f, 5.0f));
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float FloatSlider::CalcDesiredWidth() const noexcept
	{
		const float grabSize = ImGui::GetStyle().GrabMinSize;
		const float padding = ImGui::GetStyle().FramePadding.x * 2.0f;
		const float valueTextWidth = ImGui::CalcTextSize(m_Format.c_str()).x + padding;

		return valueTextWidth + grabSize + 8.0f;
	}

	Vector2 FloatSlider::ReportSize() const noexcept
	{
		ImFont* pFont = m_Style.GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;

		char valueBuffer[64];
		const float value = m_ValueCallback.IsSet() ? m_ValueCallback() : 0.0f;
		ImFormatString(valueBuffer, sizeof(valueBuffer), m_Format.c_str(), value);

		const float rawWidth = ImGui::CalcTextSize(valueBuffer).x;
		float width = rawWidth + padding.x;
		width = Math::Max(width, 100.0f);

		if (pFont)
			ImGui::PopFont();

		return { width, frameHeight };
	}

	void FloatSlider::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void FloatSlider::OnRender() noexcept
	{
		if (m_IsActive)
			SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		float value = m_ValueCallback();
		m_IsUsing = ImGui::SliderFloat("##SliderFloat", &value, m_Min, m_Max, m_Format.c_str(), GetFlags());

		if (m_IsUsing)
			m_OnChanged(value);

		m_IsHovered = ImGui::IsItemHovered();
		if (m_IsHovered || m_IsActive)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);

		const bool isActive = ImGui::IsItemActive();

		if (isActive && !m_IsActive)
			SetActive(true);
		else if (!isActive && m_IsActive)
			SetActive(false);

		if (m_IsHovered)
			ImGui::SetMouseCursor(ImGuiMouseCursor_::ImGuiMouseCursor_ResizeEW);
	}

	void FloatSlider::SetFormat(const char* pFormat) noexcept
	{
		m_Format = pFormat;
	}

	void FloatSlider::SetHandleColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SliderGrab, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void FloatSlider::SetHandleSize(float size) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_GrabMinSize, size);
	}

	void FloatSlider::SetMinValue(float value) noexcept
	{
		m_Min = value;
	}

	void FloatSlider::SetMaxValue(float value) noexcept
	{
		m_Max = value;
	}

	void FloatSlider::SetActive(bool state) noexcept
	{
		OnActiveChanged(state);
		m_IsActive = state;
	}
}
