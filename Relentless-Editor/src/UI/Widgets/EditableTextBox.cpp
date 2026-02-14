#include "EditableTextBox.h"

namespace Relentless
{
	namespace SearchBarEx_private
	{
		constexpr const Color SearchbarBackgroundColor = Color(17.0f, 17.0f, 17.0f, 255.0f);
		constexpr const Color SearchbarBorderColor = Color(60.0f, 60.0f, 60.0f, 200.0f);
		constexpr const Color SearchbarActiveColor = Color(30.0f, 120.0f, 255.0f, 200.0f);
		constexpr const Color SearchbarHoveredColor = Color(100.0f, 100.0f, 100.0f, 200.0f);
	}

	EditableTextBox::EditableTextBox(std::string_view aHintText) noexcept
		:m_InputBuffer{'\0'}
		,m_HintText{aHintText}
	{
		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);

		SetBackgroundColor(Colors::Normalize(17.0f, 17.0f, 17.0f, 255.0f));
		SetBorderColor(Colors::Normalize(SearchBarEx_private::SearchbarBackgroundColor));
	}

	void EditableTextBox::OnRender() noexcept
	{
		const Color borderCol = m_IsActive ? SearchBarEx_private::SearchbarActiveColor : m_IsHovered ? SearchBarEx_private::SearchbarHoveredColor : SearchBarEx_private::SearchbarBorderColor;
		SetBorderColor(Colors::Normalize(borderCol));
		
		Vector2 size = Vector2::Zero;
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();

		if (horizontalSizePolicy == ESizePolicy::Auto)
			size.x = 200.0f;
		else if (horizontalSizePolicy == ESizePolicy::Fixed)
			size.x = GetFixedWidth();
		else //Stretch
			size.x = GetAssignedSize().x;

		if (verticalSizePolicy == ESizePolicy::Auto)
			size.y = ImGui::GetFrameHeight();
		else if (verticalSizePolicy == ESizePolicy::Fixed)
			size.y = GetFixedHeight();
		else //Stretch
			size.y = GetAssignedSize().y;

		const bool inputDone = ImGui::InputTextEx("##EditableTextBox", m_HintText.c_str(), m_InputBuffer, IM_ARRAYSIZE(m_InputBuffer), ImVec2(size.x, size.y), ImGuiInputTextFlags_None);

		m_IsActive = ImGui::IsItemActive();

		if (inputDone)
			m_OnTextChanged.ExecuteIfSet(m_InputBuffer);

		if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsItemDeactivated())
		{
			const ETextCommitType type = Keyboard::IsKeyDown(RLS_Key::Enter) ? ETextCommitType::OnEnter : ETextCommitType::OnUserMovedFocus;
			m_OnTextCommitted.ExecuteIfSet(m_InputBuffer, type);
		}
	}

	void EditableTextBox::SetText(const String& aText) noexcept
	{
		const size_t maxLen = sizeof(m_InputBuffer) - 1;
		const size_t len = Math::Min(aText.size(), maxLen);

		std::memcpy(m_InputBuffer, aText.data(), len);
		m_InputBuffer[len] = '\0';
	}

	Vector2 EditableTextBox::ReportSize() const noexcept
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
			size.x = 200.0f;
		if (!fixedHeight)
			size.y = frameHeight;

		if (pFont)
			ImGui::PopFont();

		return size;
	}

	bool EditableTextBox::RequiresAssignedSize() const noexcept
	{
		return GetHorizontalSizePolicy() == ESizePolicy::Stretch || GetVerticalSizePolicy() == ESizePolicy::Stretch;
	}
}