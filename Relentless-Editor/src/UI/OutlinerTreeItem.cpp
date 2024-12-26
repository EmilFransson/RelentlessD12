#include "OutlinerTreeItem.h"

namespace Relentless
{
	OutlinerTreeItem::OutlinerTreeItem(ETreeItemType type, TreeItem* pParent) noexcept
		:TreeItem(pParent),
		m_Type{type}
	{
	}

	void OutlinerTreeItem::SetVisibility(bool visibilityState) noexcept
	{
		m_IsVisible = visibilityState;
	}

	void OutlinerTreeItem::SetVisibilityIcon(const AssetHandle& iconHandle) noexcept
	{
		SetColumnIcon(iconHandle, 0u);
	}

	ETreeItemType OutlinerTreeItem::GetType() const noexcept
	{
		return m_Type;
	}

	bool OutlinerTreeItem::IsVisible() const noexcept
	{
		return m_IsVisible;
	}

	OutlinerEntityTreeItem::OutlinerEntityTreeItem(entity id) noexcept
		: 
		OutlinerTreeItem(ETreeItemType::Entity),
		m_EntityID{ id }
	{
	}

	entity OutlinerEntityTreeItem::GetEntityID() const noexcept
	{
		return m_EntityID;
	}

	OutlinerSceneTreeItem::OutlinerSceneTreeItem() noexcept
		: 
		OutlinerTreeItem(ETreeItemType::Scene)
	{
	}

	OutlinerFilterTreeItem::OutlinerFilterTreeItem(const std::string& path) noexcept
		:
		OutlinerTreeItem(ETreeItemType::Filter),
		m_Path{ path }
	{
	}

	void OutlinerFilterTreeItem::SetPath(const std::string& newPath) noexcept
	{
		m_Path = newPath;
	}

	const std::string& OutlinerFilterTreeItem::GetPath() const noexcept
	{
		return m_Path;
	}
}
