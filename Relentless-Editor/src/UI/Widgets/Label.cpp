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
		return m_TextProvider.IsSet() ? m_TextProvider() : String();
	}

	//void Label::OnRender() noexcept
	//{
	//	ImGui::AlignTextToFramePadding();
	//
	//	if (!m_HighlightedSubstring.empty())
	//		RenderHighlight();
	//	
	//	RenderText();
	//}

	void Label::OnRender() noexcept
	{
		const String  text = GetText();
		const Vector2 pad = GetPadding();
		const float   framePadY = ImGui::GetStyle().FramePadding.y; // what AlignTextToFramePadding gave you

		ImVec2 origin = ImGui::GetCursorScreenPos();
		origin.x += pad.x;
		origin.y += pad.y/* + framePadY*/;                              // restore the old vertical baseline

		const float wrapWidth = ImGui::GetContentRegionAvail().x - pad.x * 2.0f;

		BuildLineLayout(text, wrapWidth, origin);

		if (!m_HighlightedSubstring.empty())
			RenderHighlight();

		RenderText(wrapWidth, origin, pad, framePadY);
	}

	void Label::BuildLineLayout(const String& aText, float aWrapWidth, ImVec2 aOrigin) noexcept
	{
		m_Lines.clear();

		ImFont* pFont = ImGui::GetFont();
		const float fontSize = ImGui::GetFontSize();
		const float lineHeight = ImGui::GetTextLineHeight();

		const char* s = aText.c_str();
		const char* textEnd = s + aText.size();
		float y = aOrigin.y;

		auto pushLine = [&](const char* b, const char* e)
			{
				const float w = pFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, b, e).x;
				float x = aOrigin.x;
				if (m_TextAlign == ETextAlign::Center) x += (aWrapWidth - w) * 0.5f;
				else if (m_TextAlign == ETextAlign::Right) x += (aWrapWidth - w);
				m_Lines.push_back({ b, e, x, y });
				y += lineHeight;
			};

		if (!m_Wrap)
		{
			pushLine(s, textEnd);
			return;
		}

		const char* lineStart = s;
		while (lineStart < textEnd)
		{
			const char* lineEnd =
				pFont->CalcWordWrapPositionA(1.0f, lineStart, textEnd, aWrapWidth);
			if (lineEnd == lineStart) ++lineEnd;        // force forward progress
			pushLine(lineStart, lineEnd);
			lineStart = lineEnd;
			while (lineStart < textEnd && *lineStart == ' ')  ++lineStart;
			if (lineStart < textEnd && *lineStart == '\n')    ++lineStart;
		}
	}

	Vector2 Label::ReportSize() const noexcept
	{
		ImFont* pFont = m_Style.GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;
		const ImVec2 textSize = ImGui::CalcTextSize(GetText().c_str());

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
		return this;
	}

	Label* Label::SetText(Callback<String()>&& aTextCallback) noexcept
	{
		m_TextProvider = std::move(aTextCallback);
		return this;
	}

	Label* Label::SetWrap(bool aWrap) noexcept
	{
		m_Wrap = aWrap;
		return this;
	}

	//void Label::RenderHighlight() noexcept
	//{
	//	const String text = GetText();
	//	const String loweredText = StringUtils::ToLower(text);
	//	const size_t startIndex = loweredText.find(m_HighlightedSubstring);
	//
	//	const ImVec2 textPos = ImGui::GetCursorScreenPos();
	//
	//	// Text before highlight
	//	const std::string before = text.substr(0, startIndex);
	//	const std::string highlight = text.substr(startIndex, m_HighlightedSubstring.length());
	//
	//	ImVec2 preSize = ImGui::CalcTextSize(before.c_str());
	//	ImVec2 highlightSize = ImGui::CalcTextSize(highlight.c_str());
	//
	//	ImVec2 highlightStart = textPos;
	//	highlightStart.x += preSize.x;
	//	highlightStart.y += 4.0f;
	//
	//	ImVec2 highlightEnd = highlightStart;
	//	highlightEnd.x += highlightSize.x;
	//	highlightEnd.y += highlightSize.y * 1.1f;
	//
	//	ImU32 highlightColor = ImGui::GetColorU32(ImVec4(0.0f, 0.6f, 0.0f, 1.0f));
	//	ImDrawList* drawList = ImGui::GetWindowDrawList();
	//	drawList->AddRectFilled(highlightStart, highlightEnd, highlightColor, 2.0f);
	//}

	void Label::RenderHighlight() noexcept
	{
		ImFont* pFont = ImGui::GetFont();
		const float fontSize = ImGui::GetFontSize();
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 color = ImGui::GetColorU32(ImVec4(0.0f, 0.6f, 0.0f, 1.0f));

		for (const LineLayout& line : m_Lines)
		{
			const String lineText(line.begin, line.end);
			const size_t idx = StringUtils::ToLower(lineText).find(m_HighlightedSubstring);
			if (idx == String::npos) continue;

			const char* matchBegin = line.begin + idx;
			const char* matchEnd = matchBegin + m_HighlightedSubstring.size();

			const float preW = pFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, line.begin, matchBegin).x;
			const float matchW = pFont->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, matchBegin, matchEnd).x;

			const ImVec2 a(line.x + preW, line.y + 2.0f);
			const ImVec2 b(a.x + matchW, line.y + fontSize + 2.0f);
			dl->AddRectFilled(a, b, color, 2.0f);
		}
	}

	//void Label::RenderText() noexcept
	//{
	//	if (m_Wrap)
	//		ImGui::TextWrapped("%s", GetText().c_str());
	//	else
	//		ImGui::Text("%s", GetText().c_str());
	//
	//	if (!this->m_IsHovered && ImGui::IsItemHovered())
	//		this->OnMouseEnter_private();
	//	else if (this->m_IsHovered && !ImGui::IsItemHovered())
	//		this->OnMouseExit_private();
	//}

	void Label::RenderText(float aWrapWidth, ImVec2 aOrigin, Vector2 aPad, float aFramePadY) noexcept
	{
		ImDrawList* dl = ImGui::GetWindowDrawList();
		const ImU32 color = ImGui::GetColorU32(ImGuiCol_Text);
		const float lineHeight = ImGui::GetTextLineHeight();

		for (const LineLayout& line : m_Lines)
			dl->AddText(ImVec2(line.x, line.y), color, line.begin, line.end);

		const float textHeight = m_Lines.empty()
			? lineHeight
			: (m_Lines.back().y + lineHeight - aOrigin.y);

		// Reserve the SAME footprint the old path did:
		//   width  = content + widget padding on both sides
		//   height = text + widget padding + the frame-padding the old item carried
		ImGui::Dummy(ImVec2(aWrapWidth + aPad.x * 2.0f,
			textHeight + aPad.y * 2.0f + aFramePadY * 2.0f));

		if (!m_IsHovered && ImGui::IsItemHovered())
			OnMouseEnter_private();
		else if (m_IsHovered && !ImGui::IsItemHovered())
			OnMouseExit_private();
	}
}


