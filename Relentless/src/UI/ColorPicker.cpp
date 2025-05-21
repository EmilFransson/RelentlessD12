#include "ColorPicker.h"

namespace Relentless
{
	ColorPicker::ColorPicker(std::string_view id, const Vector2& size, int flags) noexcept
		:IStylableWidget{id}
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

		Color color = m_ValueCallback();
		if (ImGui::ColorButton(m_ID.c_str(), ImVec4(color.R(), color.G(), color.B(), color.A()), GetFlags(), ImVec2(width, 0.0f)))
			ImGui::OpenPopup("ColorPickerPopup");

		m_IsHovered = ImGui::IsItemHovered();

		if (ImGui::BeginPopup("ColorPickerPopup"))
		{
			if (ImGui::ColorPicker4("##picker", &color.x, m_PickerFlags) && m_OnChanged.IsSet())
				m_OnChanged(color);

			ImGui::EndPopup();
		}
	}

	void ColorPicker::SetColorPickerFlags(int flags) noexcept
	{
		m_PickerFlags = flags;
	}
}
