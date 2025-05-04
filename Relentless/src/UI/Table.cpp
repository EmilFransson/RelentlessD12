#include "Table.h"

namespace Relentless
{
	Table::Table(std::string_view id) noexcept
		: IWidget{id}
	{
		SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoPadOuterX);
	}

	void Table::Add(Ref<IWidget> pWidget, uint32 column, uint32 row) noexcept
	{
		RLS_ASSERT(!HasWidget(pWidget), "[Table::Add] Widget already assigned as child.");
		m_Cells[{column, row}].push_back(pWidget);

		m_NumRows = Math::Max(m_NumRows, row + 1);
		m_NumColumns = Math::Max(m_NumColumns, column + 1);
	}

	bool Table::HasWidget(Ref<IWidget> pWidget) const noexcept
	{
		return std::any_of(m_Cells.begin(), m_Cells.end(), [&](const auto& cell)
		{
				return std::find(cell.second.begin(), cell.second.end(), pWidget) != cell.second.end();
		});
	}

	void Table::OnRender() noexcept
	{
		if (m_Cells.empty())
			return;

		SetColorsAndStyles();
		const bool isValid = ImGui::BeginTable(m_ID.c_str(), m_NumColumns, GetFlags());

		if (isValid)
		{
			ImGui::BeginGroup();
			for (uint32 row = 0u; row < m_NumRows; ++row)
			{
				ImGui::TableNextRow();

				for (uint32 column = 0u; column < m_NumColumns; ++column)
				{
					ImGui::TableSetColumnIndex(column);

					if (column == 0u && row == 0u)
						ImGui::Indent(40.0f);

					float maxItemWidth = 180.0f;     // max desired width
					float horizontalPadding = 4.0f;  // space from edge
					// Width available in the current column
					float available = ImGui::GetContentRegionAvail().x;
					// Clamp: never larger than max, never smaller than what's available
					float itemWidth = ImMin(available - horizontalPadding, maxItemWidth);
					itemWidth = ImMax(itemWidth, 0.0f); // Safety clamp

					for (auto& pWidget : m_Cells[{column, row}])
					{
						if (pWidget)
						{
							ImGui::SetNextItemWidth(itemWidth);
							pWidget->Render();
						}
					}

					ImRect rect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);

					if (column == 0u)
						rect.Max.y += (rect.Max.y - rect.Min.y) + 20.0f;

					if (ImGui::IsMouseHoveringRect(rect.Min, rect.Max))
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, ImGui::GetColorU32(ImGuiCol_Header));
				}
			}

			ImGui::EndGroup();
			ImGui::EndTable();
		}
		
		DiscardAllStylesAndColors();
	}

	void Table::SetColorsAndStyles() noexcept
	{
		SetStyleColors
		({
			{ImGuiCol_Separator, ImVec4(0, 0, 0, 255)},
			{ImGuiCol_SeparatorHovered, ImVec4(90, 90, 90, 255)},
			{ImGuiCol_SeparatorActive, ImVec4(110, 110, 110, 255)},
			{ImGuiCol_TableBorderLight, ImVec4(20.0f / 255.0f, 20.0f / 255.0f, 20.0f / 255.0f, 255)}
		});

		SetStyleVars({{ImGuiStyleVar_CellPadding, ImVec2(20.0f, 4.0f)}});
	}
}