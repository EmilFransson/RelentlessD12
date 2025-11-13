#include "ColorPicker.h"

namespace Relentless
{
	ColorPicker::ColorPicker(const Vector2& size, int flags) noexcept
		: m_Size{size}
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

		const float frameHeight = ImGui::GetFrameHeight();

		const float width = GetSizePolicy() == ESizePolicy::Stretch ? ImGui::CalcItemWidth() : (m_Size.x > 0.0f) ? m_Size.x : frameHeight;
		const float height = (m_Size.y > 0.0f) ? m_Size.y : frameHeight;

		Color color = m_ValueCallback();
		if (ImGui::ColorButton("##ColorButton", ImVec4(color.R(), color.G(), color.B(), color.A()), GetFlags(), ImVec2(width, height)))
			ImGui::OpenPopup("ColorPickerPopup");

		m_IsHovered = ImGui::IsItemHovered();

		if (ImGui::BeginPopup("ColorPickerPopup"))
		{
			if (ImGui::ColorPicker4("##picker", &color.x, m_PickerFlags) && m_OnChanged.IsSet())
				m_OnChanged(color);

			ImGui::EndPopup();
		}
	}

	Vector2 ColorPicker::ReportSize() const noexcept
	{
		const float frameHeight = ImGui::GetFrameHeight();
		const float width = (m_Size.x > 0.0f) ? m_Size.x : frameHeight;
		const float height = (m_Size.y > 0.0f) ? m_Size.y : frameHeight;

		return { width, height };
	}

	void ColorPicker::SetColorPickerFlags(int flags) noexcept
	{
		m_PickerFlags = flags;
	}
}
