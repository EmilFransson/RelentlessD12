#include "Label.h"

namespace Relentless
{
	Label::Label(std::string_view id) noexcept
		: IWidget{ id }, m_Text{id}
	{
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


