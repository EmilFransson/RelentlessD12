#include "SectionRow.h"

namespace Relentless
{
	SectionRow::SectionRow(StringView aText) noexcept
		: m_Text{ aText }
	{
	}

	float SectionRow::GetSeparatorThickness() const noexcept
	{
		return m_SeparatorThickness;
	}

	void SectionRow::OnRender() noexcept
	{
		ImGui::SeparatorText(m_Text.c_str());
	}

	Vector2 SectionRow::ReportSize() const noexcept
	{
		ImFont* pFont = m_Style.GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;
		const ImVec2 textSize = ImGui::CalcTextSize(m_Text.c_str());

		if (pFont)
			ImGui::PopFont();

		return Vector2(textSize.x + padding.x, frameHeight + m_SeparatorThickness);
	}

	SectionRow* SectionRow::SetSeparatorColor(const Color& aColor) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Separator, ImVec4(aColor.R(), aColor.G(), aColor.B(), aColor.A()));
		return this;
	}

	SectionRow* SectionRow::SetSeparatorThickness(float aThickness) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_SeparatorTextBorderSize, aThickness);
		m_SeparatorThickness = aThickness;
		return this;
	}
}