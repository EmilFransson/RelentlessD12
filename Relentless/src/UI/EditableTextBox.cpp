#include "EditableTextBox.h"

#include "Input/Keyboard.h"

namespace Relentless
{
	namespace SearchBarEx_private
	{
		constexpr const Color SearchbarBackgroundColor = Color(17.0f, 17.0f, 17.0f, 255.0f);
		constexpr const Color SearchbarBorderColor = Color(60.0f, 60.0f, 60.0f, 200.0f);
		constexpr const Color SearchbarActiveColor = Color(30.0f, 120.0f, 255.0f, 200.0f);
		constexpr const Color SearchbarHoveredColor = Color(100.0f, 100.0f, 100.0f, 200.0f);
	}

	EditableTextBox::EditableTextBox(const Vector2& aSize, std::string_view aHintText) noexcept
		:m_InputBuffer{'\0'}
		,m_Size{aSize}
		,m_HintText{aHintText}
	{
		SetFrameRounding(10.0f);
		SetBorderSize(2.0f);
		SetPadding({ ImGui::GetStyle().FramePadding.x, ImGui::GetStyle().FramePadding.y + 3.0f });

		SetBackgroundColor(Colors::Normalize(17.0f, 17.0f, 17.0f, 255.0f));
		SetBorderColor(Colors::Normalize(SearchBarEx_private::SearchbarBackgroundColor));
	}

	float EditableTextBox::CalcDesiredWidth() const noexcept
	{
		return m_Size.x;
	}

	void EditableTextBox::OnRender() noexcept
	{
		const Color borderCol = m_IsActive ? SearchBarEx_private::SearchbarActiveColor : m_IsHovered ? SearchBarEx_private::SearchbarHoveredColor : SearchBarEx_private::SearchbarBorderColor;
		SetBorderColor(Colors::Normalize(borderCol));

		const float frameHeight = ImGui::GetFrameHeight();

		const float width = GetSizePolicy() == ESizePolicy::Stretch ? ImGui::CalcItemWidth() : (m_Size.x > 0.0f) ? m_Size.x : frameHeight;
		const float height = (m_Size.y > 0.0f) ? m_Size.y : frameHeight;

		const bool inputDone = ImGui::InputTextEx("##EditableTextBox", m_HintText.c_str(), m_InputBuffer, IM_ARRAYSIZE(m_InputBuffer), ImVec2(width, height), ImGuiInputTextFlags_None);

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
		std::strncpy(m_InputBuffer, aText.c_str(), sizeof(m_InputBuffer));
		m_InputBuffer[sizeof(m_InputBuffer) - 1] = '\0';
	}

	Vector2 EditableTextBox::ReportSize() const noexcept
	{
		const float frameHeight = ImGui::GetFrameHeight();
		const float width = (m_Size.x > 0.0f) ? m_Size.x : frameHeight;
		const float height = (m_Size.y > 0.0f) ? m_Size.y : frameHeight;

		return { width, height };
	}
}