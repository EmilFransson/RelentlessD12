#include "OutlinerTable.h"

namespace Relentless
{

	Outliner::Outliner(const char* id, const std::shared_ptr<TreeDataView>& dataView) noexcept
		: Tree(id, dataView)
	{
		{
			ColumnProperties column;
			column.Label = "";
			column.HeaderTooltip = "Visibility";
			column.HeaderIcon.IconTextureHandle = NULL_HANDLE;
			column.HeaderIcon.Tint = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			column.HeaderIcon.SizeWeight = 0.6f;
			column.Flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort;
			column.DefaultWeight = 30.0f;
			column.AllowSelection = false;
			column.Alignment = UI::Alignment::Center;
			AddColumn(column);
		}
	
		{
			ColumnProperties column;
			column.Label = "Item Label";
			column.HeaderTooltip = "Item Label";
			column.Flags = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort;
			column.IsTreeNode = true;
			column.Alignment = UI::Alignment::Left;
			AddColumn(column);
		}
		
		{
			ColumnProperties column;
			column.Label = "Type";
			column.HeaderTooltip = "Displays the name of each entity's type";
			column.Flags = ImGuiTableColumnFlags_WidthStretch;
			AddColumn(column);
		}
	
		SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Sortable);
	}
}

