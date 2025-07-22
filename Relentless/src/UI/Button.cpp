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
		return m_Size.x;
	}

	const String& Button::GetText() const noexcept
	{
		return m_Text;
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
