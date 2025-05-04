#include "Button.h"

namespace Relentless
{
	Button::Button(std::string_view id, const Vector2& size) noexcept
		: IWidget{ id }
		,m_Size{size}
	{
	}

	void Button::OnRender() noexcept
	{
		SetColorsAndStyles();

		if (ImGui::Button(m_ID.c_str(), ImVec2(m_Size.x, m_Size.y)))
			OnClicked();

		DiscardAllStylesAndColors();
	}

	void Button::SetActiveColor(const Color& color) noexcept
	{
		m_Color = color;
	}

	void Button::SetColor(const Color& color) noexcept
	{
		m_Color = color;
	}

	void Button::SetHoverColor(const Color& color) noexcept
	{
		m_Color = color;
	}

	void Button::SetSize(const Vector2& size) noexcept
	{
		m_Size = size;
	}

	void Button::SetColorsAndStyles() noexcept
	{
		SetStyleColors
		({
			{ImGuiCol_Button,			ImVec4(m_Color.R(), m_Color.G(), m_Color.B(), m_Color.A())},
			{ImGuiCol_ButtonHovered,	ImVec4(m_HoverColor.R(), m_HoverColor.G(), m_HoverColor.B(), m_HoverColor.A())},
			{ImGuiCol_ButtonActive,		ImVec4(m_ActiveColor.R(), m_ActiveColor.G(), m_ActiveColor.B(), m_ActiveColor.A())},
		});

		SetStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	}

}
