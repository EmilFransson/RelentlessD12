#include "OutlinerTable.h"
#include "OutlinerTableData.h"
#include "OutlinerTableDataSelection.h"
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

		SetTableDataSelection(std::make_shared<OutlinerTableDataSelection>(this));
	}

	void OutlinerTable::OnSceneChanged(Scene* pScene) noexcept
	{
		m_pScene = pScene;
		
		ClearAllSelections();
		RemoveAll();

		Table::AddEntry(std::make_shared<OutlinerSceneTableData>(m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle));
		AddEntityEntry(0);
		AddEntityEntry(m_pScene->CreateEntity("Entity1"));
		AddEntityEntry(m_pScene->CreateEntity("Entity2"));
	}

	//TODO: forming proper hierarchy
	void OutlinerTable::AddEntityEntry(entity e) noexcept
	{
		std::shared_ptr<OutlinerEntityTableData> pTableData = std::make_shared<OutlinerEntityTableData>(e, m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle);

		std::unique_ptr<TableDataSlice>& pSlice = GetRootEntries()[0]->GetSlice();
		if (!pSlice) [[unlikely]]
			pSlice = std::make_unique<OutlinerTableDataSlice>(this, pTableData.get());

		pSlice->Add(std::move(pTableData));

		m_NrOfEntityEntries++;
	}


	void OutlinerTable::SelectAllExpandedEntityRows() noexcept
	{
		static_pointer_cast<OutlinerTableDataSelection>(GetTableDataSelection())->SelectAllExpandedEntityRows();
	}

	uint32_t OutlinerTable::GetNrOfEntityEntries() const noexcept
	{
		return m_NrOfEntityEntries;
	}

	uint32_t OutlinerTable::GetNrOfSelectedEntities() const noexcept
	{
		return static_pointer_cast<OutlinerTableDataSelection>(GetTableDataSelection())->GetNrOfSelectedEntities();
	}

	Scene* OutlinerTable::GetScene() noexcept
	{
		return m_pScene;
	}

	const char* OutlinerTable::GetID() const noexcept
	{
		return "OutlinerTable";
	}
}

