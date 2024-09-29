#include "OutlinerTable.h"
#include "OutlinerTableData.h"
#include "OutlinerTableDataSlice.h"

namespace Relentless
{
	OutlinerTable::OutlinerTable() noexcept
	{
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\showicon.rasset", m_ShowEntityTextureIconHandle), "Core engine icon missing.");
		RLS_VERIFY(AssetManager::RequestLoadAsset(std::string(ENGINE_ASSET_DIRECTORY) + "Textures\\Icons\\hideicon.rasset", m_HideEntityTextureIconHandle), "Core engine icon missing.");

		{
			ColumnProperties column;
			column.Name = "";
			column.HeaderTooltip = "Visibility";
			column.HeaderIcon.IconTextureHandle = m_ShowEntityTextureIconHandle;
			column.HeaderIcon.Tint = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			column.HeaderIcon.SizeWeight = { 0.6f, 0.6f };
			column.Flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;
			column.DefaultWeight = 30.0f;
			column.AllowSelection = false;
			column.Alignment = UI::Alignment::Center;
			AddColumn(column);
		}

		{
			ColumnProperties column;
			column.Name = "Item Label";
			column.HeaderTooltip = "Item Label";
			column.Flags = ImGuiTableColumnFlags_WidthStretch;
			column.IsTreeNode = true;
			column.HeaderIcon.IconTextureHandle = m_ShowEntityTextureIconHandle;
			column.HeaderIcon.Tint = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
			column.HeaderIcon.SizeWeight = { 0.6f, 0.6f };
			column.Alignment = UI::Alignment::Left;
			AddColumn(column);
		}
		
		{
			ColumnProperties column;
			column.Name = "Type";
			column.HeaderTooltip = "Displays the name of each entity's type";
			column.Flags = ImGuiTableColumnFlags_WidthStretch;
			AddColumn(column);
		}

		SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody);
	}

	void OutlinerTable::AddEntry(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		if (!std::dynamic_pointer_cast<OutlinerSceneTableData>(pTableData))
		{
			std::unique_ptr<TableDataSlice>& pSlice = GetRootEntries()[0]->GetSlice();
			if (!pSlice) [[unlikely]]
				pSlice = std::make_unique<OutlinerTableDataSlice>(this, pTableData.get());
				
			pSlice->Add(pTableData);
		}
		else
			Table::AddEntry(pTableData);
	}

	void OutlinerTable::OnSceneChanged(Scene* pScene) noexcept
	{
		m_pScene = pScene;
		
		RemoveAll();

		AddEntry(std::make_shared<OutlinerSceneTableData>(m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle));
		AddEntry(std::make_shared<OutlinerEntityTableData>(0, m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle));
		AddEntry(std::make_shared<OutlinerEntityTableData>(m_pScene->CreateEntity("Entity1"), m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle));
		AddEntry(std::make_shared<OutlinerEntityTableData>(m_pScene->CreateEntity("Entity2"), m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle));
	}

	const char* OutlinerTable::GetID() const noexcept
	{
		return "OutlinerTable";
	}
}

