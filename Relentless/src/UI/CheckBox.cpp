#include "CheckBox.h"

namespace Relentless
{
	CheckBox::CheckBox(std::string_view id) noexcept
		: IWidget(id)
	{
	}

	void CheckBox::OnRender() noexcept
	{
		ImGui::GetWindowDrawList()->ChannelsSplit(2);

		auto curPos = ImGui::GetCursorScreenPos();
		ImGui::SetCursorScreenPos(ImVec2(curPos.x, curPos.y + ImGui::GetFrameHeightWithSpacing() - ImGui::GetFrameHeight()));
		curPos = ImGui::GetCursorScreenPos();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

			SetColorsAndStyles();

			if (ImGui::Checkbox(m_ID.c_str(), &m_State))
				OnCheckStateChanged(m_State);

			m_Hovered = ImGui::IsItemHovered();
		
			DiscardAllStylesAndColors();
		}
		
		const ImVec2 size = ImGui::GetItemRectSize();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

			const ImVec2 min = ImVec2(curPos.x - 2, curPos.y - 2);
			const ImVec2 max = ImVec2(min.x + size.x + 4.0f, min.y + size.y + 4.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, m_Hovered ? IM_COL32(75, 75, 75, 255) : IM_COL32(50, 50, 50, 255), 3);
		}

		ImGui::GetWindowDrawList()->ChannelsMerge();
	}

	void CheckBox::SetColorsAndStyles() noexcept
	{
		SetStyleColors
		({
			{ImGuiCol_FrameBg, IM_COL32(15.0f, 15.0f, 15.0f, 255.0f)},
			{ImGuiCol_FrameBgHovered, IM_COL32(15.0f, 15.0f, 15.0f, 255.0f)},
			{ImGuiCol_FrameBgActive, IM_COL32(15.0f, 15.0f, 15.0f, 255.0f)}
		});

		SetStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		SetStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
	}

}
