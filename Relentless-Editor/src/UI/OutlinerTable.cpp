#include "OutlinerTable.h"
#include "OutlinerTableDataSelection.h"
#include "OutlinerTableDataSlice.h"
#include "OutlinerTableInteraction.h"
#include "OutlinerTableStyling.h"

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
			column.Flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize | ImGuiTableColumnFlags_NoSort;
			column.DefaultWeight = 30.0f;
			column.AllowSelection = false;
			column.Alignment = UI::Alignment::Center;
			AddColumn(column);
		}

		{
			ColumnProperties column;
			column.Name = "Item Label";
			column.HeaderTooltip = "Item Label";
			column.Flags = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_DefaultSort;
			column.IsTreeNode = true;
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

		SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_Sortable);

		SetTableDataSelection(std::make_shared<OutlinerTableDataSelection>(this));
		SetTableInteraction(std::make_shared<OutlinerTableInteraction>(this));
	}

	void OutlinerTable::OnSceneChanged(Scene* pScene) noexcept
	{
		RLS_ASSERT(pScene, "[OutlinerTable]: Scene is invalid.");
		m_pScene = pScene;

		SetTableStyling(std::make_shared<OutlinerTableStyling>(GetTableDataSelection().get(), m_pScene));

		ClearAllSelections();
		RemoveAll();

		AddEntry(std::make_shared<OutlinerSceneTableData>(this, m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle));

		m_pScene->GetEntityManager().Collect<RootComponent>().Do([this](entity e)
			{
				AddEntityEntry(e);
			});
	}

	void OutlinerTable::AddEntityEntry(entity e, const std::unique_ptr<TableDataSlice>& pSlice) noexcept
	{
		std::shared_ptr<OutlinerEntityTableData> pTableData = std::make_shared<OutlinerEntityTableData>(this, e, m_pScene, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle);
		
		const std::vector<entity> descendants = m_pScene->GetAllEntityDescendants(e);
		for (auto& descendant : descendants)
			AddEntityEntry(descendant, pTableData->GetSlice());

		m_EntityIDToTableDataEntry[e] = pTableData;

		if (pSlice)
			pSlice->Add(std::move(pTableData));
		else
			GetRootEntries()[0]->AddChild(std::move(pTableData));
	}

	void OutlinerTable::AddFolderEntry(const char* name) noexcept
	{
		std::shared_ptr<OutlinerFolderTableData> pTableData = std::make_shared<OutlinerFolderTableData>(this, m_pScene, name, m_ShowEntityTextureIconHandle, m_HideEntityTextureIconHandle);
		GetRootEntries()[0]->AddChild(std::move(pTableData));
	}

	void OutlinerTable::SelectAllExpandedEntityRows() noexcept
	{
		static_pointer_cast<OutlinerTableDataSelection>(GetTableDataSelection())->SelectAllExpandedEntityRows();
	}

	void OutlinerTable::SelectEntity(entity e) noexcept
	{
		RLS_ASSERT(m_EntityIDToTableDataEntry.contains(e), "[OutlinerTable]: Entity does not exist in table!");

		const std::shared_ptr<TableDataSelection>& pTableDataSelection = GetTableDataSelection();
		if (!pTableDataSelection->IsSelected(m_EntityIDToTableDataEntry.at(e)))
			pTableDataSelection->Select(m_EntityIDToTableDataEntry.at(e));
	}

	void OutlinerTable::DeselectEntity(entity e) noexcept
	{
		RLS_ASSERT(m_EntityIDToTableDataEntry.contains(e), "[OutlinerTable]: Entity does not exist in table!");

		const std::shared_ptr<TableDataSelection>& pTableDataSelection = GetTableDataSelection();
		if (pTableDataSelection->IsSelected(m_EntityIDToTableDataEntry.at(e)))
			pTableDataSelection->Deselect(m_EntityIDToTableDataEntry.at(e));
	}

	void OutlinerTable::DeselectAllEntities() noexcept
	{
		static_pointer_cast<OutlinerTableDataSelection>(GetTableDataSelection())->DeselectAllEntities();
	}

	bool OutlinerTable::IsEntitySelected(entity e) const noexcept
	{
		RLS_ASSERT(m_EntityIDToTableDataEntry.contains(e), "[OutlinerTable]: Entity does not exist in table!");

		const std::shared_ptr<TableDataSelection>& pTableDataSelection = GetTableDataSelection();
		return pTableDataSelection->IsSelected(m_EntityIDToTableDataEntry.at(e));
	}

	uint32_t OutlinerTable::GetNrOfEntityEntries() const noexcept
	{
		return m_EntityIDToTableDataEntry.size();
	}

	uint32_t OutlinerTable::GetNrOfSelectedEntities() const noexcept
	{
		return static_pointer_cast<OutlinerTableDataSelection>(GetTableDataSelection())->GetNrOfSelectedEntities();
	}

	const std::vector<entity>& OutlinerTable::GetSelectedEntities() const noexcept
	{
		return static_pointer_cast<OutlinerTableDataSelection>(GetTableDataSelection())->GetSelectedEntities();
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

