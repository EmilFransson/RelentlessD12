#include "ComboBox.h"

namespace Relentless
{
	ComboBox::ComboBox(int flags) noexcept
	{
		SetFlags(flags);

		//Frame
		const Color color = Colors::Normalize(12.5f, 12.5f, 12.5f, 255.0f);
		SetBackgroundColor(color);
		SetHoverColor(color);
		SetActiveColor(color);

		//Button:
		SetDropDownButtonColor(color);
		SetDropDownButtonActiveColor(color);
		SetDropDownButtonHoveredColor(color);

		//Selectables:
		SetSelectableBackgroundColor(Colors::Normalize(66.0f, 150.0f, 250.0f, 255.0f));

		SetFrameRounding(6.0f);
		SetBorderSize(2.0f);
		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	ComboBox* ComboBox::AddSelectables(Span<const char*> selectables) noexcept
	{
		if (selectables.GetSize() == 0u)
			return this;

		m_Selectables = selectables.Copy();
		m_CurrentSelection.Name = m_Selectables[0];

		return this;
	}

	ComboBox* ComboBox::Bind(PropertyHandle<int>* aPropertyHandle) noexcept
	{
		m_pPropertyHandle = aPropertyHandle;
		return this;
	}

	const char* ComboBox::GetSelectedItem() const
	{
		return m_CurrentSelection.Name;
	}

	int ComboBox::GetSelectedIndex() const
	{
		return m_CurrentSelection.Index;
	}

	void ComboBox::SetDropDownButtonColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Button, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void ComboBox::SetDropDownButtonActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void ComboBox::SetDropDownButtonHoveredColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	ComboBox* ComboBox::SetSelectedItem(const char* pItem) noexcept
	{
		for (uint32 i = 0u; i < m_Selectables.size(); ++i)
		{
			if (strcmp(pItem, m_Selectables[i]) == 0)
			{
				m_CurrentSelection.Index = i;
				m_CurrentSelection.Name = m_Selectables[i];
				break;
			}
		}

		return this;
	}

	ComboBox* ComboBox::SetSelectedItem(int aIndex) noexcept
	{
		if (static_cast<int>(m_Selectables.size()) < aIndex)
			return this;

		m_CurrentSelection.Index = aIndex;
		m_CurrentSelection.Name = m_Selectables[aIndex];

		return this;
	}

	void ComboBox::SetSelectableBackgroundColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void ComboBox::OnRender() noexcept
	{
		const char* preview = m_CurrentSelection.Name;
		if (m_Selectables.empty())
			preview = "-";
		else if (m_pPropertyHandle)
		{
			int index = 0;
			const EPropertyAccessResult accessResult = m_pPropertyHandle->GetValue(index);

			if (accessResult == EPropertyAccessResult::Success)
			{
				index = Math::Clamp(index, 0, static_cast<int>(m_Selectables.size()) - 1);
				if (index != m_CurrentSelection.Index)
				{
					m_CurrentSelection.Index = index;
					m_CurrentSelection.Name = m_Selectables[index];
					preview = m_CurrentSelection.Name;
				}
			}
			else if (accessResult == EPropertyAccessResult::MixedValues)
				preview = "Mixed";
		}

		if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		if (ImGui::BeginCombo("##ComboBox", preview))
		{
			for (size_t i = 0u; i < m_Selectables.size(); ++i)
			{
				const bool isSelected = m_CurrentSelection.Index == static_cast<int>(i);
				if (ImGui::Selectable(m_Selectables[i], isSelected))
				{
					if (!isSelected)
					{
						m_CurrentSelection.Index = static_cast<int>(i);
						m_CurrentSelection.Name = m_Selectables[m_CurrentSelection.Index];

						if (m_pPropertyHandle)
							m_pPropertyHandle->SetValue(i);
						else
							m_OnSelectionChanged.ExecuteIfSet(m_CurrentSelection);
					}
				}

				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}

			ImGui::EndCombo();
		}

		m_IsHovered = ImGui::IsItemHovered();
	}

	Vector2 ComboBox::ReportSize() const noexcept
	{
		const ESizePolicy horizontalSizePolicy = GetHorizontalSizePolicy();
		const ESizePolicy verticalSizePolicy = GetVerticalSizePolicy();
		const bool fixedWidth = horizontalSizePolicy == ESizePolicy::Fixed;
		const bool fixedHeight = verticalSizePolicy == ESizePolicy::Fixed;

		Vector2 size = Vector2::Zero;

		if (fixedWidth)
			size.x = GetFixedWidth();
		if (fixedHeight)
			size.y = GetFixedHeight();

		if (fixedWidth && fixedHeight)
			return size;

		ImFont* pFont = GetStyle().GetFont();
		if (pFont) 
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;
		const float frameHeight = ImGui::GetFontSize() + padding.y;
		
		if (!fixedWidth)
			size.x = 150.0f;
		if (!fixedHeight)
			size.y = frameHeight;

		if (pFont) 
			ImGui::PopFont();
		
		return size;
	}
}
