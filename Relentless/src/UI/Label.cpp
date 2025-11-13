#include "Label.h"

#include "Utility/StringUtils.h"

namespace Relentless
{
	Label::Label(std::string_view text, ImFont* pFont) noexcept
		: m_Text{text}
	{
		if (pFont)
			SetFont(pFont);
		else
			SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float Label::CalcDesiredWidth() const noexcept
	{
		return ImGui::CalcTextSize(m_Text.c_str()).x;
	}

	const String& Label::GetText() const noexcept
	{
		return m_Text;
	}

	void Label::OnRender() noexcept
	{
		ImGui::AlignTextToFramePadding();

		if (!m_HighlightedSubstring.empty())
			RenderHighlight();
		
		RenderText();
	}

	Vector2 Label::ReportSize() const noexcept
	{
		ImFont* pFont = m_Style.GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;
		const ImVec2 textSize = ImGui::CalcTextSize(m_Text.c_str());

		if (pFont)
			ImGui::PopFont();

		return Vector2(textSize.x + padding.x, frameHeight);
	}

	Label* Label::SetHighlightedSubstring(std::string_view text) noexcept
	{
		m_HighlightedSubstring = StringUtils::ToLower(String(text));
		return this;
	}

	Label* Label::SetText(std::string_view text) noexcept
	{
		m_Text = text;
		return this;
	}

	void Label::RenderHighlight() noexcept
	{
		const String loweredText = StringUtils::ToLower(m_Text);
		const size_t startIndex = loweredText.find(m_HighlightedSubstring);

		const ImVec2 textPos = ImGui::GetCursorScreenPos();

		// Text before highlight
		const std::string before = m_Text.substr(0, startIndex);
		const std::string highlight = m_Text.substr(startIndex, m_HighlightedSubstring.length());

		ImVec2 preSize = ImGui::CalcTextSize(before.c_str());
		ImVec2 highlightSize = ImGui::CalcTextSize(highlight.c_str());

		ImVec2 highlightStart = textPos;
		highlightStart.x += preSize.x;
		highlightStart.y += 4.0f;

		ImVec2 highlightEnd = highlightStart;
		highlightEnd.x += highlightSize.x;
		highlightEnd.y += highlightSize.y * 1.1f;

		ImU32 highlightColor = ImGui::GetColorU32(ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddRectFilled(highlightStart, highlightEnd, highlightColor, 2.0f);
	}

	void Label::RenderText() noexcept
	{
		ImGui::Text(m_Text.c_str());

		if (!this->m_IsHovered && ImGui::IsItemHovered())
			this->OnMouseEnter_private();
		else if (this->m_IsHovered && !ImGui::IsItemHovered())
			this->OnMouseExit_private();
	}

}


