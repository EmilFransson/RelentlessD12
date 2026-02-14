#include "ColorPicker.h"

namespace Relentless
{
	ColorPicker::ColorPicker(int flags) noexcept
	{
		SetFlags(flags);

		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	void ColorPicker::OnRender() noexcept
	{
		constexpr Color HOVER_BORDER_COLOR = Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f);
		constexpr Color BORDER_COLOR = Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f);

		if (m_IsHovered)
			SetBorderColor(HOVER_BORDER_COLOR);
		else
			SetBorderColor(BORDER_COLOR);

		const Vector2 size = GetRenderedSize();

		Color color = m_ValueCallback.IsSet() ? m_ValueCallback() : Colors::White;
		if (ImGui::ColorButton("##ColorButton", ImVec4(color.R(), color.G(), color.B(), color.A()), GetFlags(), ImVec2(size.x, size.y)))
			ImGui::OpenPopup("ColorPickerPopup");

		m_IsHovered = ImGui::IsItemHovered();

		if (ImGui::BeginPopup("ColorPickerPopup"))
		{
			if (ImGui::ColorPicker4("##picker", &color.x, m_PickerFlags) && m_OnChanged.IsSet())
				m_OnChanged.ExecuteIfSet(color);

			ImGui::EndPopup();
		}
	}

	Vector2 ColorPicker::GetRenderedSize() const noexcept
	{
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();

		Vector2 size = Vector2::Zero;
		if (horizontalSizePolicy == ESizePolicy::Fixed)
			size.x = GetFixedWidth();
		else if (horizontalSizePolicy == ESizePolicy::Stretch)
			size.x = GetAssignedSize().x;
		else
			size.x = 200.0f;

		if (verticalSizePolicy == ESizePolicy::Fixed)
			size.y = GetFixedHeight();
		else if (verticalSizePolicy == ESizePolicy::Stretch)
			size.y = GetAssignedSize().y;
		else
			size.y = ImGui::GetFrameHeight();
	
		return size;
	}

	Vector2 ColorPicker::ReportSize() const noexcept
	{
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();
		Vector2 size = Vector2::Zero;

		if (horizontalSizePolicy == ESizePolicy::Fixed)
			size.x = GetFixedWidth();
		else
			size.x = 200.0f;

		if (verticalSizePolicy == ESizePolicy::Fixed)
			size.y = GetFixedHeight();
		else
			size.y = ImGui::GetFrameHeight();

		return size;
	}

	bool ColorPicker::RequiresAssignedSize() const noexcept
	{
		return GetHorizontalSizePolicy() == ESizePolicy::Stretch || GetVerticalSizePolicy() == ESizePolicy::Stretch;
	}

	void ColorPicker::SetColorPickerFlags(int flags) noexcept
	{
		m_PickerFlags = flags;
	}
}
