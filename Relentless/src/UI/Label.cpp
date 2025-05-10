#include "Label.h"

namespace Relentless
{
	Label::Label(std::string_view id, ImFont* pFont) noexcept
		: IStylableWidget{ id }, m_Text{id}
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

	void Label::OnRender() noexcept
	{
		ImGui::AlignTextToFramePadding();
		ImGui::Text(m_Text.c_str());
	}

	void Label::SetText(std::string_view text) noexcept
	{
		m_Text = text;
	}
}


