#include "OutlinerTableData.h"
#include "OutlinerTableDataSlice.h"

namespace Relentless
{
	OutlinerTableData::OutlinerTableData(Table* pTable, ETableDataType type) noexcept
		:
		TableData{pTable},
		m_Type{type}
	{

	}


	ETableDataType OutlinerTableData::GetType() const noexcept
	{
		return m_Type;
	}

	OutlinerEntityTableData::OutlinerEntityTableData(Table* pTable, entity id, Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept
		: 
		OutlinerTableData(pTable, ETableDataType::Entity),
		m_EntityID{ id },
		m_pOwningScene{ pScene },
		m_ShowEntityTextureIconHandle{ showTextureIconHandle },
		m_HideEntityTextureIconHandle{ hideTextureIconHandle }
	{
		SetTableDataSlice(std::make_unique<OutlinerTableDataSlice>(pTable, this));
	}

	const char* OutlinerEntityTableData::GetColumnString(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "";
		case 1: return m_pOwningScene->GetEntityManager().Get<NameComponent>(m_EntityID).Name.c_str();
		case 2: return "Entity";
		default: return "";
		}
	}

	const char* OutlinerEntityTableData::GetColumnTooltip(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "Toggles the visibility of this entity in the level editor.";
		default: return "";
		}
	}

	AssetHandle OutlinerEntityTableData::GetColumnIcon(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0:
		{
			if (IsVisible())
				return m_ShowEntityTextureIconHandle;
			else
				return m_HideEntityTextureIconHandle;
		}
		default:
			return NULL_HANDLE;
		}
	}

	bool OutlinerEntityTableData::IsVisible() const noexcept
	{
		if (m_pOwningScene->GetEntityManager().Has<HiddenInGameComponent>(m_EntityID))
			return false;
		else
			return true;
	}

	void OutlinerEntityTableData::SetAndPropagateVisibleState(bool visibleState) noexcept
	{
		const bool isVisible = IsVisible();

		if (visibleState == isVisible)
			return;

		if (!isVisible)
			m_pOwningScene->GetEntityManager().Remove<HiddenInGameComponent>(m_EntityID);
		else
			m_pOwningScene->GetEntityManager().AddOrReplace<HiddenInGameComponent>(m_EntityID);
	
		const auto& pSlice = GetConstSlice();
		const std::vector<std::shared_ptr<TableData>>& children = pSlice->GetData();

		for (auto& child : children)
			static_cast<OutlinerTableData*>(child.get())->SetAndPropagateVisibleState(visibleState);
	}

	entity OutlinerEntityTableData::GetEntityID() const noexcept
	{
		return m_EntityID;
	}

	OutlinerSceneTableData::OutlinerSceneTableData(Table* pTable, Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept
		: 
		OutlinerTableData(pTable, ETableDataType::Scene),
		m_pScene{ pScene },
		m_ShowEntityTextureIconHandle{ showTextureIconHandle },
		m_HideEntityTextureIconHandle{ hideTextureIconHandle }
	{
		SetTableDataSlice(std::make_unique<OutlinerTableDataSlice>(pTable, this));
	}

	const char* OutlinerSceneTableData::GetColumnString(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "";
		case 1: return m_pScene->GetName().c_str();
		case 2: return "Scene";
		default: return "";
		}
	}

	const char* OutlinerSceneTableData::GetColumnTooltip(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "Toggles the visibility of all entities in the level editor.";
		default: return "";
		}
	}

	AssetHandle OutlinerSceneTableData::GetColumnIcon(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0:
		{
			if (IsVisible())
				return m_ShowEntityTextureIconHandle;
			else
				return m_HideEntityTextureIconHandle;
		}
		default:
			return NULL_HANDLE;
		}
	}

	bool OutlinerSceneTableData::IsVisible() const noexcept
	{
		if (m_pScene->GetEntityManager().GetEntityCountForPool<HiddenInGameComponent>() == m_pScene->GetEntityManager().GetEntityAliveCount())
			return false;
		else
			return true;
	}

	void OutlinerSceneTableData::SetAndPropagateVisibleState(bool visibleState) noexcept
	{
		const bool isVisible = IsVisible();

		if (isVisible == visibleState)
			return;

		const auto& pSlice = GetConstSlice();
		const std::vector<std::shared_ptr<TableData>>& children = pSlice->GetData();
		for (auto& child : children)
			static_cast<OutlinerTableData*>(child.get())->SetAndPropagateVisibleState(visibleState);
	}

	Scene* OutlinerSceneTableData::GetScene() noexcept
	{
		return m_pScene;
	}

	OutlinerFolderTableData::OutlinerFolderTableData(Table* pTable, Scene* pScene, const char* name, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept
		:
		OutlinerTableData(pTable, ETableDataType::Folder),
		m_pScene{pScene},
		m_Name{name},
		m_ShowEntityTextureIconHandle{ showTextureIconHandle },
		m_HideEntityTextureIconHandle{ hideTextureIconHandle }
	{
		SetTableDataSlice(std::make_unique<OutlinerTableDataSlice>(pTable, this));
	}

	const char* OutlinerFolderTableData::GetColumnString(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "";
		case 1: return m_Name.c_str();
		case 2: return "Folder";
		default: return "";
		}
	}

	const char* OutlinerFolderTableData::GetColumnTooltip(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "Toggles the visibility of all entities in the level editor.";
		default: return "";
		}
	}

	AssetHandle OutlinerFolderTableData::GetColumnIcon(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0:
		{
			if (IsVisible())
				return m_ShowEntityTextureIconHandle;
			else
				return m_HideEntityTextureIconHandle;
		}
		default:
			return NULL_HANDLE;
		}
	}

	bool OutlinerFolderTableData::IsVisible() const noexcept
	{
		if (!HasChildren())
			return false;

		const std::unique_ptr<TableDataSlice>& pSlice = GetConstSlice();
		const std::vector<std::shared_ptr<TableData>>& tableDatas = pSlice->GetData();

		return std::all_of(tableDatas.begin(), tableDatas.end(), [](const std::shared_ptr<TableData>& pTableData) -> bool
			{
				return static_cast<OutlinerTableData*>(pTableData.get())->IsVisible();
			});
	}

	void OutlinerFolderTableData::SetAndPropagateVisibleState(bool visibleState) noexcept
	{
		const bool isVisible = IsVisible();

		if (visibleState == isVisible)
			return;
	
		const auto& pSlice = GetConstSlice();
		const std::vector<std::shared_ptr<TableData>>& children = pSlice->GetData();

		for (auto& child : children)
			static_cast<OutlinerTableData*>(child.get())->SetAndPropagateVisibleState(visibleState);
	}

}
