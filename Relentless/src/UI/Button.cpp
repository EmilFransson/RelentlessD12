#include "Button.h"

namespace Relentless
{
	Button::Button(std::string_view text, const Vector2& size) noexcept
		:m_Text{text}
		,m_Size{size}
	{
		SetBackgroundColor(Colors::Normalize(56.0f, 56.0f, 56.0f, 255.0f));
		SetBorderColor(Colors::Normalize(20.0f, 20.0f, 20.0f, 255.0f));

		SetBorderSize(2.0f);
		SetFrameRounding(4.0f);
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float Button::CalcDesiredWidth() const noexcept
	{
		return ImGui::CalcTextSize(m_Text.c_str()).x + (ImGui::GetStyle().FramePadding.x * 2.0f);
	}

	const String& Button::GetText() const noexcept
	{
		return m_Text;
	}

	Vector2 Button::ReportSize() const noexcept
	{
		ImFont* pFont = GetStyle().GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const ImVec2 textSize = ImGui::CalcTextSize(m_Text.c_str(), nullptr, false);
		const Vector2 padding = GetPadding() * 2.0f;

		const float lineHeight = ImGui::GetFontSize();
		const float textHeight = (m_Text.empty() ? lineHeight : textSize.y);

		Vector2 sizeToReport(textSize.x + padding.x, textHeight + padding.y);

		if (m_Size.x > 0.0f) 
			sizeToReport.x = Math::Max(sizeToReport.x, m_Size.x);
		if (m_Size.y > 0.0f) 
			sizeToReport.y = Math::Max(sizeToReport.y, m_Size.y);

		if (pFont) 
			ImGui::PopFont();

		return sizeToReport;
	}

	Button* Button::SetActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	Button* Button::SetBackgroundColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Button, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	Button* Button::SetHoverColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	void Button::OnRender() noexcept
	{
		if (ImGui::Button(m_Text.c_str(), ImVec2(m_Size.x, m_Size.y)))
			m_OnClickedCallback();

		if (!this->m_IsHovered && ImGui::IsItemHovered())
			this->OnMouseEnter_private();
		else if (this->m_IsHovered && !ImGui::IsItemHovered())
			this->OnMouseExit_private();
	}

	void Button::SetSize(const Vector2& size) noexcept
	{
		m_Size = size;
	}

	void Button::SetText(const String& text) noexcept
	{
		m_Text = text;
	}

}
