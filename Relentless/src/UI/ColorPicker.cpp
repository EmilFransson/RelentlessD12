#include "ColorPicker.h"

namespace Relentless
{
	ColorPicker::ColorPicker(std::string_view id, const Color& initialColor, const Vector2& size, int flags) noexcept
		:IStylableWidget{id}
		,m_Color{initialColor}
		,m_Size{size}
	{
		SetFlags(flags);

		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetBorderSize(2.0f);
		SetFrameRounding(3.0f);
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	float ColorPicker::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	void ColorPicker::OnRender() noexcept
	{
		if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		const float width = Math::Min(ImGui::GetContentRegionAvail().x, m_Size.x);

		if (ImGui::ColorButton(m_ID.c_str(), ImVec4(m_Color.R(), m_Color.G(), m_Color.B(), m_Color.A()), GetFlags(), ImVec2(width, 0.0f)))
			ImGui::OpenPopup("ColorPickerPopup");

		m_IsHovered = ImGui::IsItemHovered();

		if (ImGui::BeginPopup("ColorPickerPopup"))
		{
			if (ImGui::ColorPicker4("##picker", &m_Color.x, m_PickerFlags))
				OnChanged(m_Color);

			ImGui::EndPopup();
		}
	}

	void ColorPicker::SetColorPickerFlags(int flags) noexcept
	{
		m_PickerFlags = flags;
	}
}
