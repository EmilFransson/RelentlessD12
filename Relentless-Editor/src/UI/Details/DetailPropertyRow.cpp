#include "DetailPropertyRow.h"
#include "UI/Widgets/Label.h"

namespace Relentless
{
	DetailPropertyRow::DetailPropertyRow() noexcept
	{
		m_pResetToDefaultSlot = new Label(ICON_FA_ARROW_LEFT, ImGui::GetIO().Fonts->Fonts[2]);
	}

	void DetailPropertyRow::OnRender() noexcept
	{
		ImGui::TableNextRow();

		const int row = ImGui::TableGetRowIndex();

		constexpr float maxItemWidth = 180.0f;     // max desired width
		constexpr float horizontalPadding = 4.0f;  // space from edge

		{
			ImGui::TableSetColumnIndex(0);
			
			if (row == 0u)
				ImGui::Indent(40.0f);

			const float available = ImGui::GetContentRegionAvail().x;
			const float itemWidth = Math::Max(0.0f, Math::Min(available - horizontalPadding, maxItemWidth));
			ImGui::SetNextItemWidth(itemWidth);

			m_pNameContentSlot->Render();

			ImRect rect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 0);
			rect.Max.y += (rect.Max.y - rect.Min.y) + 20.0f;

			if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Header));
		}
		
		{
			ImGui::TableSetColumnIndex(1);

			const float available = ImGui::GetContentRegionAvail().x;
			const float itemWidth = Math::Max(0.0f, Math::Min(available - horizontalPadding, maxItemWidth));
			ImGui::SetNextItemWidth(itemWidth);

			m_pValueContentSlot->Render();

			ImRect rect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 1);

			if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Header));
		}
		
		{
			ImGui::TableSetColumnIndex(2);

			const float available = ImGui::GetContentRegionAvail().x;
			const float itemWidth = Math::Max(0.0f, Math::Min(available - horizontalPadding, maxItemWidth));
			
			ImGui::SetNextItemWidth(itemWidth);

			m_pResetToDefaultSlot->Render();

			ImRect rect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), 2);

			if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
				ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Header));
		}
	}
}
