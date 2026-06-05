#include "RadioButton.h"

namespace Relentless
{
	RadioButton::RadioButton(StringView aText) noexcept
		: m_Text{ aText }
	{
		SetBackgroundColor(Colors::Normalize(56.0f, 56.0f, 56.0f, 255.0f));
		SetBorderColor(Colors::Normalize(20.0f, 20.0f, 20.0f, 255.0f));

		SetBorderSize(2.0f);
		SetFrameRounding(4.0f);
	}

	void RadioButton::OnRender() noexcept
	{
		const bool value = m_ValueCallback.IsSet() ? m_ValueCallback() : false;
		if (ImGui::RadioButton(m_Text.c_str(), value) && value == false)
			m_OnValueChanged.ExecuteIfSet(value);
	}

	Vector2 RadioButton::ReportSize() const noexcept
	{
		Vector2 size = Vector2::Zero;
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();
		const bool fixedWidth = horizontalSizePolicy == ESizePolicy::Fixed;
		const bool fixedHeight = verticalSizePolicy == ESizePolicy::Fixed;

		if (fixedWidth)
			size.x = this->GetFixedWidth();
		if (fixedHeight)
			size.y = this->GetFixedHeight();

		if (fixedWidth && fixedHeight)
			return size;

		ImFont* pFont = GetStyle().GetFont();
		if (pFont)
			ImGui::PushFont(pFont);

		const ImGuiStyle& style = ImGui::GetStyle();

		const ImVec2 textSize = ImGui::CalcTextSize(m_Text.c_str(), nullptr, false);
		const Vector2 padding = GetPadding() * 2.0f;

		const float square = ImGui::GetFrameHeight();

		float contentWidth = square;
		if (!m_Text.empty())
			contentWidth += style.ItemInnerSpacing.x + textSize.x;

		const float contentHeight = Math::Max(square, m_Text.empty() ? square : textSize.y);

		if (!fixedWidth)
			size.x = contentWidth + padding.x;
		if (!fixedHeight)
			size.y = contentHeight + padding.y;

		if (pFont)
			ImGui::PopFont();

		return size;
	}
}
