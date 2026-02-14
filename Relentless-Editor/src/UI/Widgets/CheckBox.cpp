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

	void CheckBox::OnRender() noexcept
	{
		bool state = m_ValueCallback.IsSet() ? m_ValueCallback() : false;
		if (ImGui::Checkbox("##Checkbox", &state))
			m_OnCheckStateChanged.ExecuteIfSet(state);

		m_Hovered = ImGui::IsItemHovered();
		if (m_Hovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
	}

	Vector2 CheckBox::ReportSize() const noexcept
	{
		Vector2 size = Vector2::Zero;
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();
		const bool fixedWidth = horizontalSizePolicy == ESizePolicy::Fixed;
		const bool fixedHeight = verticalSizePolicy == ESizePolicy::Fixed;

		if (fixedWidth)
			size.x = GetFixedWidth();
		if (fixedHeight)
			size.y = GetFixedHeight();

		ImFont* pFont = GetStyle().GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;

		if (!fixedWidth)
			size.x = frameHeight;
		if (!fixedHeight)
			size.y = frameHeight;

		if (pFont)
			ImGui::PopFont();

		return size;
	}
}
