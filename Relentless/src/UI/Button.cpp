#include "Button.h"

namespace Relentless
{
	Button::Button(std::string_view id, const Vector2& size) noexcept
		: IStylableWidget{ id }
		, m_Size{size}
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

	void Button::SetActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void Button::SetBackgroundColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Button, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void Button::SetHoverColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void Button::OnRender() noexcept
	{
		if (ImGui::Button(m_ID.c_str(), ImVec2(m_Size.x, m_Size.y)))
			OnClicked();
	}

	void Button::SetSize(const Vector2& size) noexcept
	{
		m_Size = size;
	}
}
