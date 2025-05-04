#include "ColorPicker.h"

namespace Relentless
{
	ColorPicker::ColorPicker(std::string_view id, const Color& initialColor, const Vector2& size, int flags) noexcept
		:IWidget{id}
		,m_Color{initialColor}
		,m_Size{size}
	{
		SetFlags(flags);
	}

	void ColorPicker::OnRender() noexcept
	{
		ImGui::GetWindowDrawList()->ChannelsSplit(2);

		auto curPos = ImGui::GetCursorScreenPos();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(1);

			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

			constexpr float horizontalPadding = 4.0f;
			const float width = Math::Min(ImGui::GetContentRegionAvail().x - horizontalPadding, m_Size.x);

			if (ImGui::ColorButton(m_ID.c_str(), ImVec4(m_Color.R(), m_Color.G(), m_Color.B(), m_Color.A()), GetFlags(), ImVec2(width, m_Size.y)))
				ImGui::OpenPopup("ColorPickerPopup");

			m_IsHovered = ImGui::IsItemHovered();
			ImGui::PopStyleVar();
		}

		const ImVec2 size = ImGui::GetItemRectSize();

		{
			ImGui::GetWindowDrawList()->ChannelsSetCurrent(0);

			const ImVec2 min = ImVec2(curPos.x - 2, curPos.y - 2);
			const ImVec2 max = ImVec2(min.x + size.x + 4.0f, min.y + size.y + 4.0f);
			ImGui::GetWindowDrawList()->AddRectFilled(min, max, m_IsHovered ? IM_COL32(75, 75, 75, 255) : IM_COL32(50, 50, 50, 255), 6);
		}


		ImGui::GetWindowDrawList()->ChannelsMerge();

		if (ImGui::BeginPopup("ColorPickerPopup"))
		{
			if (ImGui::ColorPicker4("##picker", &m_Color.x, m_PickerFlags))
				OnChanged(m_Color);

			ImGui::EndPopup();
		}
	}

	void ColorPicker::SetColorPickerFlags(int flags) noexcept
	{
		m_PickerFlags = flags;
	}
}
