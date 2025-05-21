#include "IntSlider.h"
#include "Input/Mouse.h"

namespace Relentless
{
	IntSlider::IntSlider(std::string_view id, int min, int max, const char* pFormat, int flags) noexcept
		:IStylableWidget{ id }
		, m_Format{ pFormat }
		, m_Min{ min }
		, m_Max{ max }
	{
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
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float IntSlider::CalcDesiredWidth() const noexcept
	{
		const float grabSize = ImGui::GetStyle().GrabMinSize;
		const float padding = ImGui::GetStyle().FramePadding.x * 2.0f;

		float valueTextWidth = ImGui::CalcTextSize(m_Format.c_str()).x + padding;
		float labelTextWidth = 0.0f;

		// Only include label width if it's visible (not just an ID)
		const char* visibleLabel = ImGui::FindRenderedTextEnd(m_ID.c_str());
		if (visibleLabel[0] != '\0')
			labelTextWidth = ImGui::CalcTextSize(m_ID.c_str()).x + padding;

		// Final width: label + value + grab + spacing
		return labelTextWidth + valueTextWidth + grabSize + 8.0f;
	}

	void IntSlider::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void IntSlider::OnRender() noexcept
	{
		if (m_IsActive)
			SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		int value = m_ValueCallback();
		m_IsUsing = ImGui::SliderInt(m_ID.c_str(), &value, m_Min, m_Max, m_Format.c_str(), GetFlags());

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

	void IntSlider::SetFormat(const char* pFormat) noexcept
	{
		m_Format = pFormat;
	}

	void IntSlider::SetHandleColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SliderGrab, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void IntSlider::SetHandleSize(float size) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_GrabMinSize, size);
	}

	void IntSlider::SetMinValue(float value) noexcept
	{
		m_Min = value;
	}

	void IntSlider::SetMaxValue(float value) noexcept
	{
		m_Max = value;
	}

	void IntSlider::SetActive(bool state) noexcept
	{
		OnActiveChanged(state);
		m_IsActive = state;
	}
}
