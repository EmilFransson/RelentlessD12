#include "ComboBox.h"

namespace Relentless
{
	ComboBox::ComboBox(std::string_view id, int flags) noexcept
		: IWidget{ id }
	{
		SetFlags(flags);
	}

	void ComboBox::AddSelectables(Span<const char*> selectables) noexcept
	{
		m_Selectables = selectables.Copy();
	}

	int ComboBox::GetSelectedIndex() const
	{
		return m_Selected;
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
		
		ImGui::GetWindowDrawList()->ChannelsSplit(2);

		SetColorsAndStyles();
		
		auto curPos = ImGui::GetCursorScreenPos();
		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

			if (ImGui::BeginCombo(m_ID.c_str(), m_Selectables[m_Selected]))
			{
				for (int i = 0; i < m_Selectables.size(); ++i)
				{
					const bool isSelected = m_Selected == i;
					if (ImGui::Selectable(m_Selectables[i], isSelected))
					{
						if (m_Selected != i)
						{
							m_Selected = i;
							OnChanged(m_Selected);
						}
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}

			m_IsHovered = ImGui::IsItemHovered();

			DiscardAllStylesAndColors();
		}

		const ImVec2 size = ImGui::GetItemRectSize();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

			const ImVec2 min = ImVec2(curPos.x - 2, curPos.y - 2);
			const ImVec2 max = ImVec2(min.x + size.x + 4.0f, min.y + size.y + 4.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, m_IsHovered ? IM_COL32(75, 75, 75, 255) : IM_COL32(50, 50, 50, 255), 6);
		}

		ImGui::GetWindowDrawList()->ChannelsMerge();
	}

	void ComboBox::SetColorsAndStyles() noexcept
	{
		SetStyleColors
		({
			{ImGuiCol_FrameBg, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_FrameBgHovered, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_FrameBgActive, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_Button, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_ButtonHovered, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_ButtonActive, ImVec4(0.05f, 0.05f, 0.05f, 1.0f)},
			{ImGuiCol_HeaderHovered, ImVec4(66.0f / 255.0f, 150.0f / 250.0f, 250.0f / 255.0f, 1.0f)}
		});

		SetStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
		SetStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 5.0f));
	}

}


