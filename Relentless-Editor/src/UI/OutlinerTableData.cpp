#include "OutlinerTableData.h"

namespace Relentless
{
	OutlinerEntityTableData::OutlinerEntityTableData(entity id, Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept
		: 
		m_EntityID{ id },
		m_pOwningScene{ pScene },
		m_ShowEntityTextureIconHandle{ showTextureIconHandle },
		m_HideEntityTextureIconHandle{ hideTextureIconHandle }
	{

	}

	const char* OutlinerEntityTableData::GetColumnString(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "Icon";
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
		if (m_pOwningScene->GetEntityManager().Has<HiddenInGameComponent>(m_EntityID))
			return m_HideEntityTextureIconHandle;
		else
			return m_ShowEntityTextureIconHandle;
	}

	OutlinerSceneTableData::OutlinerSceneTableData(Scene* pScene, const AssetHandle& showTextureIconHandle, const AssetHandle& hideTextureIconHandle) noexcept
		: 
		m_pScene{ pScene },
		m_ShowEntityTextureIconHandle{ showTextureIconHandle },
		m_HideEntityTextureIconHandle{ hideTextureIconHandle }
	{
	}

	const char* OutlinerSceneTableData::GetColumnString(uint32_t columnIndex) const noexcept
	{
		switch (columnIndex)
		{
		case 0: return "Icon";
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
		if (m_pScene->GetEntityManager().GetEntityCountForPool<HiddenInGameComponent>() == m_pScene->GetEntityManager().GetEntityAliveCount())
			return m_HideEntityTextureIconHandle;
		else
			return m_ShowEntityTextureIconHandle;
	}
}
