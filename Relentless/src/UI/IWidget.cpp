#include "IWidget.h"

namespace Relentless
{
	void WidgetStyle::Apply() noexcept
	{
		const ImVec2 pos = ImGui::GetCursorPos();
		if (m_Margin.Left > 0.0f) 
			ImGui::SetCursorPosX(pos.x + m_Margin.Left);
		if (m_Margin.Top > 0.0f) 
			ImGui::SetCursorPosY(pos.y + m_Margin.Top);

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

		const ImVec2 pos = ImGui::GetCursorPos();
		if (m_Margin.Right > 0.0f)
			ImGui::SetCursorPosX(pos.x + m_Margin.Right);
		if (m_Margin.Bottom > 0.0f)
			ImGui::SetCursorPosY(pos.y + m_Margin.Bottom);
	}

	void WidgetStyle::SetFont(ImFont* pFont) noexcept
	{
		m_pFont = pFont;
	}

	void WidgetStyle::SetMargin(const IntRect& margin) noexcept
	{
		m_Margin = margin;
	}

	void WidgetStyle::SetStyleVar(ImGuiStyleVar styleVar, ImVec2 value) noexcept
	{
		m_Vars1[styleVar] = value;
	}

	void WidgetStyle::SetStyleVar(ImGuiStyleVar styleVar, float value) noexcept
	{
		m_Vars2[styleVar] = value;
	}

	void WidgetStyle::SetStyleColor(ImGuiCol styleColor, ImVec4 value) noexcept
	{
		m_Cols[styleColor] = value;
	}
}