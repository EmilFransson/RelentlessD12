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
		SetPadding(Vector2(6.0f, 5.0f));
		SetBorderSize(2.0f);
		SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	ComboBox* ComboBox::AddSelectables(Span<const char*> selectables) noexcept
	{
		m_Selectables = selectables.Copy();
		return this;
	}

	float ComboBox::CalcDesiredWidth() const noexcept
	{
		float maxTextWidth = 0.0f;

		for (const auto& item : m_Selectables)
		{
			const ImVec2 size = ImGui::CalcTextSize(item);
			maxTextWidth = Math::Max(maxTextWidth, size.x);
		}

		// Add padding to account for ImGui's internal spacing and arrow icon
		const float padding = ImGui::GetStyle().FramePadding.x * 2.0f;
		const float arrowWidth = ImGui::GetFrameHeight(); // Approx width of arrow dropdown

		return maxTextWidth + padding + arrowWidth;
	}

	const char* ComboBox::GetSelectedItem() const
	{
		return m_Selectables[m_Selected];
	}

	int ComboBox::GetSelectedIndex() const
	{
		return m_Selected;
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

	ComboBox* ComboBox::SetInitiallySelectedItem(const char* pItem) noexcept
	{
		for (uint32 i = 0u; i < m_Selectables.size(); ++i)
		{
			if (strcmp(pItem, m_Selectables[i]) == 0)
			{
				m_Selected = i;
				break;
			}
		}

		return this;
	}

	void ComboBox::SetSelectableBackgroundColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_HeaderHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void ComboBox::OnPreRender() noexcept
	{
		if (!Math::AreValuesClose(m_WidthConstraint, -1.0f))
			ImGui::SetNextItemWidth(m_WidthConstraint);
	}

	void ComboBox::OnRender() noexcept
	{
		if (m_Selectables.empty())
			return;
		
		if (m_IsHovered)
			SetBorderColor(Colors::Normalize(75.0f, 75.0f, 75.0f, 255.0f));
		else
			SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));

		if (ImGui::BeginCombo("##ComboBox", m_Selectables[m_Selected]))
		{
			for (int i = 0; i < m_Selectables.size(); ++i)
			{
				const bool isSelected = m_Selected == i;
				if (ImGui::Selectable(m_Selectables[i], isSelected))
				{
					if (m_Selected != i)
					{
						m_Selected = i;
						m_OnSelectionChanged(m_Selectables[m_Selected]);
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
		if (m_Selectables.empty())
			return Vector2::Zero;

		// Match the font you actually render with
		ImFont* pFont = GetStyle().GetFont();
		if (pFont) 
			ImGui::PushFont(pFont);

		const Vector2 padding = GetPadding() * 2.0f;

		const ImGuiStyle& style = ImGui::GetStyle();
		const float frameHeight = ImGui::GetFontSize() + padding.y;
		const float arrowW = frameHeight;
		const float inner = style.ItemInnerSpacing.x;
		const float padX = padding.x;

		const float textWidth = ImGui::CalcTextSize(m_Selectables[m_Selected]).x;

		float width = textWidth + inner + arrowW + padX;
		float height = frameHeight;

		if (pFont) 
			ImGui::PopFont();
		
		return { width, height };
	}
}
