#include "Button.h"

namespace Relentless
{
	Button::Button(std::string_view aText, const Vector2& aSize) noexcept
		:m_Text{ aText }
		,m_Size{ aSize }
	{
		SetBackgroundColor(Colors::Normalize(56.0f, 56.0f, 56.0f, 255.0f));
		SetBorderColor(Colors::Normalize(20.0f, 20.0f, 20.0f, 255.0f));

		SetBorderSize(2.0f);
		SetFrameRounding(4.0f);
	}

	const String& Button::GetText() const noexcept
	{
		return m_Text;
	}

	Vector2 Button::ReportSize() const noexcept
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

		const ImVec2 textSize = ImGui::CalcTextSize(m_Text.c_str(), nullptr, false);
		const Vector2 padding = GetPadding() * 2.0f;

		const float lineHeight = ImGui::GetFontSize();
		const float textHeight = (m_Text.empty() ? lineHeight : textSize.y);

		if (!fixedWidth)
			size.x = textSize.x + padding.x;
		if (!fixedHeight)
			size.y = textHeight + padding.y;

		if (pFont) 
			ImGui::PopFont();

		return size;
	}

	Button* Button::SetActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	Button* Button::SetBackgroundColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Button, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	Button* Button::SetHoverColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
		return this;
	}

	void Button::OnRender() noexcept
	{
		const Vector2 size = GetAssignedSize();

		if (ImGui::Button(m_Text.c_str(), ImVec2(size.x, size.y)))
			m_OnClickedCallback.ExecuteIfSet();

		if (!this->m_IsHovered && ImGui::IsItemHovered())
			this->OnMouseEnter_private();
		else if (this->m_IsHovered && !ImGui::IsItemHovered())
			this->OnMouseExit_private();
	}

	void Button::SetText(const String& aText) noexcept
	{
		m_Text = aText;
	}

	bool Button::RequiresAssignedSize() const noexcept
	{
		return true;
	}
}
