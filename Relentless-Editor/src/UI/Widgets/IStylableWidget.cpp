#include "IStylableWidget.h"

namespace Relentless
{
	void WidgetStyle::Apply() noexcept
	{
		if (m_pFont)
			ImGui::PushFont(m_pFont);

		for (auto& [style, value] : m_Vars1)
			ImGui::PushStyleVar(style, value);

		for (auto& [style, value] : m_Vars2)
			ImGui::PushStyleVar(style, value);

		for (auto& [styleColor, value] : m_Cols)
			ImGui::PushStyleColor(styleColor, value);
	}

	void WidgetStyle::Discard() noexcept
	{
		ImGui::PopStyleColor(m_Cols.size());
		ImGui::PopStyleVar(m_Vars1.size() + m_Vars2.size());

		if (m_pFont)
			ImGui::PopFont();
	}

	ImFont* WidgetStyle::GetFont() const noexcept
	{
		return m_pFont;
	}

	Vector2 WidgetStyle::GetPadding() const noexcept
	{
		if (m_Vars1.contains(ImGuiStyleVar_FramePadding))
			return Vector2(m_Vars1.at(ImGuiStyleVar_FramePadding).x, m_Vars1.at(ImGuiStyleVar_FramePadding).y);

		return Vector2(ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y);
	}

	void WidgetStyle::SetFont(ImFont* aFont) noexcept
	{
		m_pFont = aFont;
	}

	void WidgetStyle::SetStyleVar(ImGuiStyleVar aStyleVar, ImVec2 aValue) noexcept
	{
		m_Vars1[aStyleVar] = aValue;
	}

	void WidgetStyle::SetStyleVar(ImGuiStyleVar aStyleVar, float aValue) noexcept
	{
		m_Vars2[aStyleVar] = aValue;
	}

	void WidgetStyle::SetStyleColor(ImGuiCol aStyleColor, ImVec4 aValue) noexcept
	{
		m_Cols[aStyleColor] = aValue;
	}
}