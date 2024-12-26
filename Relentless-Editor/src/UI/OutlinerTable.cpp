#include "OutlinerTable.h"

namespace Relentless
{

	Outliner::Outliner(const std::shared_ptr<TreeDataView>& dataView) noexcept
		: Tree(dataView)
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

	//OutlinerTable::OutlinerTable() noexcept
	//{
	//	RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\showicon.rasset", m_ShowEntityTextureIconHandle), "Core engine icon missing.");
	//	RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\hideicon.rasset", m_HideEntityTextureIconHandle), "Core engine icon missing.");
	//
	//	{
	//		ColumnProperties column;
	//		column.Name = "";
	//		column.HeaderTooltip = "Visibility";
	//		column.HeaderIcon.IconTextureHandle = m_ShowEntityTextureIconHandle;
	//		column.HeaderIcon.Tint = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	//		column.HeaderIcon.SizeWeight = { 0.6f, 0.6f };
	//		column.Flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort;
	//		column.DefaultWeight = 30.0f;
	//		column.AllowSelection = false;
	//		column.Alignment = UI::Alignment::Center;
	//		AddColumn(column);
	//	}
	//
	//	{
	//		ColumnProperties column;
	//		column.Name = "Item Label";
	//		column.HeaderTooltip = "Item Label";
	//		column.Flags = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort;
	//		column.IsTreeNode = true;
	//		column.Alignment = UI::Alignment::Left;
	//		AddColumn(column);
	//	}
	//	
	//	{
	//		ColumnProperties column;
	//		column.Name = "Type";
	//		column.HeaderTooltip = "Displays the name of each entity's type";
	//		column.Flags = ImGuiTableColumnFlags_WidthStretch;
	//		AddColumn(column);
	//	}
	//
	//	SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Sortable);
	//
	//	SetTableDataSelection(std::make_shared<OutlinerTableDataSelection>(this));
	//	SetTableInteraction(std::make_shared<OutlinerTableInteraction>(this));
	//}

	const char* Outliner::GetID() const noexcept
	{
		return "Outliner";
	}
}

