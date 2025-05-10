#include "CheckBox.h"

namespace Relentless
{
	CheckBox::CheckBox(std::string_view id) noexcept
		: IStylableWidget(id)
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
		const float frameHeight = ImGui::GetFrameHeight(); // checkbox square

		const char* visibleLabel = ImGui::FindRenderedTextEnd(m_ID.c_str());
		const bool hasText = (visibleLabel[0] != '\0');

		if (hasText)
		{
			const float spacing = ImGui::GetStyle().ItemInnerSpacing.x; // space between checkbox and label
			const float labelWidth = ImGui::CalcTextSize(m_ID.c_str()).x;
			return frameHeight + spacing + labelWidth;
		}
		else
		{
			return frameHeight;
		}
	}

	void CheckBox::OnRender() noexcept
	{
		if (ImGui::Checkbox(m_ID.c_str(), &m_State))
			OnCheckStateChanged(m_State);

		m_Hovered = ImGui::IsItemHovered();
		if (m_Hovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
	}
}
