#include "FloatEntryBox.h"

namespace Relentless
{
	FloatEntryBox::FloatEntryBox(float aStep, const char* aFormat, int someFlags) noexcept
		:m_Step{ aStep }
	{
		if (aFormat == nullptr || aFormat[0] == '\0')
			m_Format = "%.2f";
		else
			m_Format = aFormat;

		SetFlags(someFlags);
		AddFlags(ImGuiInputTextFlags_CharsNoBlank);

		const Color defaultFrameColor = Colors::Normalize(12.5f, 12.5, 12.5f, 255.0f);
		SetBackgroundColor(defaultFrameColor);
		SetHoverColor(defaultFrameColor);
		SetActiveColor(defaultFrameColor);
		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetHandleColor(Color(0.35f, 0.35f, 0.35f, 255.0f));

		SetFrameRounding(6.0f);
		SetPadding(Vector2(6.0f, 5.0f));
		SetBorderSize(2.0f);
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float FloatEntryBox::CalcDesiredWidth() const noexcept
	{
		if (GetSizePolicy() == ESizePolicy::Stretch)
			return 0.0f;

		const float grabSize = ImGui::GetStyle().GrabMinSize;
		const float padding = ImGui::GetStyle().FramePadding.x * 2.0f;

		const float valueTextWidth = ImGui::CalcTextSize(m_Format.c_str()).x + padding;
		const float indicatorWidth = m_DrawColorIndicator ? 5.0f + 6.0f : 0.0f; // rect + spacing

		return valueTextWidth + grabSize + indicatorWidth + 6.0f; // extra spacing
	}

	void FloatEntryBox::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void FloatEntryBox::OnRender() noexcept
	{
		if (m_IsActive)
			SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		auto cursorPosPreDraw = ImGui::GetCursorScreenPos();

		float value = m_ValueCallback();
		ImGui::InputFloat("##InputFloat", &value, m_Step, m_Step, m_Format.c_str(), GetFlags());

		auto cursorPosPostDraw = ImGui::GetCursorScreenPos();

		if (ImGui::IsItemDeactivated())
			m_OnChanged(value);

		m_IsHovered = ImGui::IsItemHovered();
		const bool isActive = ImGui::IsItemActive();

		if (isActive && !m_IsActive)
			SetActive(true);
		else if (!isActive && m_IsActive)
			SetActive(false);

		const ImVec2 size = ImGui::GetItemRectSize();

		if (m_DrawColorIndicator)
		{
			const ImVec2 indicatorStartLocation = ImVec2(cursorPosPreDraw.x + 3.0f, cursorPosPreDraw.y + 6.0f);
			ImGui::SetCursorScreenPos(indicatorStartLocation);

			const ImVec2 min = indicatorStartLocation;
			const ImVec2 max = ImVec2(min.x + 2.0f, min.y + size.y - 11.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(m_IndicatorColor.R(), m_IndicatorColor.G(), m_IndicatorColor.B(), m_IndicatorColor.A())), 3.0f);

			ImGui::SetCursorScreenPos(cursorPosPostDraw);
		}
	}

	Vector2 FloatEntryBox::ReportSize() const noexcept
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

	void FloatEntryBox::SetDrawColorIndicator(bool state) noexcept
	{
		m_DrawColorIndicator = state;
	}

	void FloatEntryBox::SetFormat(const char* aFormat) noexcept
	{
		m_Format = aFormat;
	}

	void FloatEntryBox::SetHandleColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SliderGrab, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void FloatEntryBox::SetHandleSize(float size) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_GrabMinSize, size);
	}

	FloatEntryBox* FloatEntryBox::SetIndicatorColor(const Color& color) noexcept
	{
		SetDrawColorIndicator(true);
		m_IndicatorColor = color;
		return this;
	}

	void FloatEntryBox::SetActive(bool state) noexcept
	{
		OnActiveChanged(state);
		m_IsActive = state;
	}
}
