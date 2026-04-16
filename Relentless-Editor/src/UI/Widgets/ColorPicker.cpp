#include "ColorPicker.h"

#include "Property/PropertyHandle.h"

namespace Relentless
{
	ColorPicker::ColorPicker(int flags) noexcept
	{
		SetFlags(flags);

		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);
	}

	ColorPicker* ColorPicker::Bind(Ref<PropertyHandle<Color>> aPropertyHandle) noexcept
	{
		m_pPropertyHandle = aPropertyHandle;
		return this;
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
		Color color = Colors::Transparent;
		EPropertyAccessResult accessResult = EPropertyAccessResult::Fail;

		if (m_pPropertyHandle)
			accessResult = m_pPropertyHandle->GetValue(color);
		if (accessResult == EPropertyAccessResult::Fail && m_ValueCallback.IsSet())
			color = m_ValueCallback();

		if (ImGui::ColorButton("##ColorButton", ImVec4(color.R(), color.G(), color.B(), color.A()), GetFlags(), ImVec2(size.x, size.y)))
			ImGui::OpenPopup("ColorPickerPopup");

		m_IsHovered = ImGui::IsItemHovered();

		if (ImGui::BeginPopup("ColorPickerPopup"))
		{
			if (ImGui::ColorPicker4("##picker", &color.x, m_PickerFlags))
			{
				if (m_pPropertyHandle)
					m_pPropertyHandle->SetValue(color);
				else
					m_OnChanged.ExecuteIfSet(color);
			}

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
