#include "Label.h"
#include "ImGui/ImGuiFonts.h"

namespace Relentless
{
	Label::Label(StringView aText, ImFont* aFont) noexcept
	{
		SetText(aText);
		SetTextColor(Colors::TextDefault);

		if (aFont)
			SetFont(aFont);
		else
			SetFont(UI::Fonts::Get("Default"));
	}

	String Label::GetText() const noexcept
	{
		return m_TextProvider.IsSet() ? m_TextProvider() : String(); //m_Text;
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
		const ImVec2 textSize = ImGui::CalcTextSize(/*m_Text*/GetText().c_str());

		if (pFont)
			ImGui::PopFont();

		return Vector2(textSize.x + padding.x, frameHeight);
	}

	Label* Label::SetHighlightedSubstring(StringView aText) noexcept
	{
		m_HighlightedSubstring = StringUtils::ToLower(String(aText));
		return this;
	}

	Label* Label::SetText(StringView aText) noexcept
	{
		m_TextProvider = [text = String(aText)]() { return text; };
		//m_Text = aText;
		return this;
	}

	Label* Label::SetText(Callback<String()>&& aTextCallback) noexcept
	{
		m_TextProvider = std::move(aTextCallback);
		return this;
	}

	void Label::RenderHighlight() noexcept
	{
		const String text = GetText();
		const String loweredText = StringUtils::ToLower(/*m_Text*/text);
		const size_t startIndex = loweredText.find(m_HighlightedSubstring);

		const ImVec2 textPos = ImGui::GetCursorScreenPos();

		// Text before highlight
		const std::string before = text.substr(0, startIndex);
		const std::string highlight = text.substr(startIndex, m_HighlightedSubstring.length());

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
		ImGui::Text("%s", GetText().c_str());

		if (!this->m_IsHovered && ImGui::IsItemHovered())
			this->OnMouseEnter_private();
		else if (this->m_IsHovered && !ImGui::IsItemHovered())
			this->OnMouseExit_private();
	}

}


