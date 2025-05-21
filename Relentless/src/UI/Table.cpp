#include "Table.h"

namespace Relentless
{
	Table::Table(std::string_view id) noexcept
		: IStylableWidget{id}
	{
		SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoPadOuterX);
	
		SetBorderLightColor(Colors::Normalize(20.0f, 20.0f, 20.0f, 255.0f));
		SetSeparatorColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
		SetSeparatorHoverColor(Colors::Normalize(90.0f, 90.0f, 90.0f, 255.0f));
		SetSeparatorActiveColor(Colors::Normalize(110.0f, 110.0f, 110.0f, 255.0f));

		SetCellPadding(Vector2(20.0f, 4.0f));
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	//IWidget* Table::Add(Ref<IWidget> pWidget, uint32 column, uint32 row) noexcept
	//{
	//	RLS_ASSERT(!HasWidget(pWidget), "[Table::Add] Widget already assigned as child.");
	//	m_Cells[{column, row}].push_back(pWidget);
	//
	//	m_NumRows = Math::Max(m_NumRows, row + 1);
	//	m_NumColumns = Math::Max(m_NumColumns, column + 1);
	//
	//	return m_Cells[{column, row}].back().Get();
	//}

	

	float Table::CalcDesiredWidth() const noexcept
	{
		if (m_Cells.empty())
			return 0.0f;

		std::vector<float> columnWidths(m_NumColumns, 0.0f);

		for (const auto& [pos, widgets] : m_Cells)
		{
			const uint32 column = pos.first;

			for (const auto& widget : widgets)
			{
				if (widget && widget->GetSizePolicy() != ESizePolicy::Stretch)
				{
					columnWidths[column] = ImMax(columnWidths[column], widget->CalcDesiredWidth());
				}
			}
		}

		float spacing = ImGui::GetStyle().CellPadding.x * 2.0f + ImGui::GetStyle().ItemSpacing.x;
		float total = 0.0f;

		for (uint32 i = 0; i < m_NumColumns; ++i)
			total += columnWidths[i];

		total += spacing * (m_NumColumns - 1); // spacing between columns

		return total;
	}

	bool Table::HasWidget(Ref<IWidget> pWidget) const noexcept
	{
		return std::any_of(m_Cells.begin(), m_Cells.end(), [&](const auto& cell)
		{
			return std::find(cell.second.begin(), cell.second.end(), pWidget) != cell.second.end();
		});
	}

	void Table::SetCellPadding(const Vector2& padding) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_CellPadding, ImVec2(padding.x, padding.y));
	}

	void Table::SetBorderLightColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_TableBorderLight, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void Table::SetSeparatorColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Separator, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void Table::SetSeparatorHoverColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SeparatorHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void Table::SetSeparatorActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SeparatorActive,  ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void Table::OnRender() noexcept
	{
		if (m_Cells.empty())
			return;

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
	}
}