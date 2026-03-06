#include "EditableTextBox.h"

#include "Property/PropertyHandle.h"

namespace Relentless
{
	namespace EditableTextBox_private
	{
		constexpr const Color BackgroundColor = Color(17.0f, 17.0f, 17.0f, 255.0f);
		constexpr const Color BorderColor = Color(60.0f, 60.0f, 60.0f, 200.0f);
		constexpr const Color ActiveColor = Color(30.0f, 120.0f, 255.0f, 200.0f);
		constexpr const Color HoveredColor = Color(100.0f, 100.0f, 100.0f, 200.0f);

		static int InputTextCallback_Resize(ImGuiInputTextCallbackData* aData)
		{
			if (aData->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				String* str = static_cast<String*>(aData->UserData);
				str->resize(static_cast<size_t>(aData->BufTextLen));
				aData->Buf = str->data();
			}

			return 0;
		}
	}

	EditableTextBox::EditableTextBox(std::string_view aHintText) noexcept
		:m_HintText{aHintText}
	{
		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);

		SetBackgroundColor(Colors::Normalize(17.0f, 17.0f, 17.0f, 255.0f));
		SetBorderColor(Colors::Normalize(EditableTextBox_private::BackgroundColor));
	}

	EditableTextBox* EditableTextBox::Bind(Ref<PropertyHandle<String>> aPropertyHandle) noexcept
	{
		m_pPropertyHandle = aPropertyHandle;
		return this;
	}

	void EditableTextBox::OnRender() noexcept
	{
		const Color borderCol = m_IsActive ? EditableTextBox_private::ActiveColor : m_IsHovered ? EditableTextBox_private::HoveredColor : EditableTextBox_private::BorderColor;
		SetBorderColor(Colors::Normalize(borderCol));
		
		const char* pPreview = m_HintText.c_str();

		if (m_pPropertyHandle)
		{
			String value;
			const EPropertyAccessResult accessResult = m_pPropertyHandle->GetValue(value);

			if (accessResult == EPropertyAccessResult::Success)
				m_Buffer = std::move(value);
			else if (accessResult == EPropertyAccessResult::MixedValues)
				pPreview = "Mixed";
		}

		constexpr int flags = ImGuiInputTextFlags_CallbackResize;

		const bool inputDone = ImGui::InputTextWithHint("##EditableTextBox", pPreview, m_Buffer.data(),
			m_Buffer.capacity() + 1, flags, EditableTextBox_private::InputTextCallback_Resize, &m_Buffer);

		m_IsActive = ImGui::IsItemActive();

		if (inputDone)
		{
			if (m_pPropertyHandle)
				m_pPropertyHandle->SetValue(m_Buffer);
			else 
				m_OnTextChanged.ExecuteIfSet(m_Buffer.c_str());
		}

		if (ImGui::IsItemDeactivatedAfterEdit() || ImGui::IsItemDeactivated())
		{
			const ETextCommitType type = Keyboard::IsKeyDown(RLS_Key::Enter) ? ETextCommitType::OnEnter : ETextCommitType::OnUserMovedFocus;
			m_OnTextCommitted.ExecuteIfSet(m_Buffer.c_str(), type);
		}
	}

	void EditableTextBox::SetText(const String& aText) noexcept
	{
		m_Buffer = aText;
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
}