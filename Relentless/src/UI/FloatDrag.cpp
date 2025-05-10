#include "FloatDrag.h"

namespace Relentless
{
	FloatDrag::FloatDrag(std::string_view id, float speed, float min, float max, const char* pFormat, int flags) noexcept
		:IStylableWidget{ id }
		,m_Speed{ speed }
		,m_Min{ min }
		,m_Max{ max }
		,m_Format{ pFormat }
	{
		SetFlags(flags);

		const Color defaultFrameColor = Colors::Normalize(12.5f, 12.5, 12.5f, 255.0f);
		SetBackgroundColor(defaultFrameColor);
		SetHoverColor(defaultFrameColor);
		SetActiveColor(defaultFrameColor);
		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetHandleColor(Color(0.35f, 0.35f, 0.35f, 255.0f));

		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);

	}

	float FloatDrag::CalcDesiredWidth() const noexcept
	{
		if (GetSizePolicy() == ESizePolicy::Stretch)
			return 0.0f;

		const float grabSize = ImGui::GetStyle().GrabMinSize;
		const float padding = ImGui::GetStyle().FramePadding.x * 2.0f;

		// Width of numeric value text (e.g. %.2f ° lux etc.)
		float valueTextWidth = ImGui::CalcTextSize(m_Format.c_str()).x + padding;

		// Optional color indicator width
		float indicatorWidth = m_DrawColorIndicator ? 5.0f + 6.0f : 0.0f; // rect + spacing

		// Label width (if visible)
		float labelTextWidth = 0.0f;
		const char* visibleLabel = ImGui::FindRenderedTextEnd(m_ID.c_str());
		if (visibleLabel[0] != '\0' && visibleLabel[0] != '#')
			labelTextWidth = ImGui::CalcTextSize(m_ID.c_str()).x + padding;

		return labelTextWidth + valueTextWidth + grabSize + indicatorWidth + 6.0f; // extra spacing
	}

	void FloatDrag::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void FloatDrag::OnRender() noexcept
	{
		if (m_IsActive)
			SetBorderColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));
		else if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		auto curPos = ImGui::GetCursorScreenPos();

		float value = m_ValueCallback();
		m_IsUsing = ImGui::DragFloat(m_ID.c_str(), &value, m_Speed, m_Min, m_Max, m_Format.c_str(), GetFlags());

		if (m_IsUsing)
			m_OnChanged(value);

		m_IsHovered = ImGui::IsItemHovered();
		const bool isActive = ImGui::IsItemActive();

		if (isActive && !m_IsActive)
			OnActiveChanged(true);
		else if (!isActive && m_IsActive)
			OnActiveChanged(false);

		m_IsActive = isActive;

		const ImVec2 size = ImGui::GetItemRectSize();

		if (m_DrawColorIndicator)
		{
			const ImVec2 indicatorStartLocation = ImVec2(curPos.x + 3.0f, curPos.y + 6.0f);
			ImGui::SetCursorScreenPos(indicatorStartLocation);

			const ImVec2 min = indicatorStartLocation;
			const ImVec2 max = ImVec2(min.x + 2.0f, min.y + size.y - 11.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(m_IndicatorColor.R(), m_IndicatorColor.G(), m_IndicatorColor.B(), m_IndicatorColor.A())), 3.0f);
		
			ImGui::SetCursorScreenPos(curPos);
		}
	}

	void FloatDrag::SetDrawColorIndicator(bool state) noexcept
	{
		m_DrawColorIndicator = state;
	}

	void FloatDrag::SetHandleColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SliderGrab, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void FloatDrag::SetHandleSize(float size) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_GrabMinSize, size);
	}

	void FloatDrag::SetIndicatorColor(const Color& color) noexcept
	{
		SetDrawColorIndicator(true);
		m_IndicatorColor = color;
	}
}
