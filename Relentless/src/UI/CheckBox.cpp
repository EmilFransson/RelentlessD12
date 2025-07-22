#include "CheckBox.h"

namespace Relentless
{
	CheckBox::CheckBox() noexcept
	{
		SetBackgroundColor(Colors::Normalize(15.0f, 15.0f, 15.0f, 255.0f));
		SetHoverColor(Colors::Normalize(15.0f, 15.0f, 15.0f, 255.0f));
		SetActiveColor(Colors::Normalize(15.0f, 15.0f, 15.0f, 255.0f));
		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		SetFrameRounding(3.0f);
		SetBorderSize(2.0f);
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float CheckBox::CalcDesiredWidth() const noexcept
	{
		return ImGui::GetFrameHeight(); // checkbox square;
	}

	void CheckBox::OnRender() noexcept
	{
		bool state = m_ValueCallback();
		if (ImGui::Checkbox("##Checkbox", &state))
			m_OnCheckStateChanged(state);

		m_Hovered = ImGui::IsItemHovered();
		if (m_Hovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
	}
}
