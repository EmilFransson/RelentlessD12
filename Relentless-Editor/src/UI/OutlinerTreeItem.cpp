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

	//void OutlinerEntityTableData::SetAndPropagateVisibleState(bool visibleState) noexcept
	//{
	//	const bool isVisible = IsVisible();
	//
	//	if (visibleState == isVisible)
	//		return;
	//
	//	if (!isVisible)
	//		m_pOwningScene->GetEntityManager().Remove<HiddenInGameComponent>(m_EntityID);
	//	else
	//		m_pOwningScene->GetEntityManager().AddOrReplace<HiddenInGameComponent>(m_EntityID);
	//
	//	const auto& pSlice = GetConstSlice();
	//	const std::vector<std::shared_ptr<TableData>>& children = pSlice->GetData();
	//
	//	for (auto& child : children)
	//		static_cast<OutlinerTableData*>(child.get())->SetAndPropagateVisibleState(visibleState);
	//}

	entity OutlinerEntityTreeItem::GetEntityID() const noexcept
	{
		return m_EntityID;
	}

	OutlinerSceneTreeItem::OutlinerSceneTreeItem(Scene* pScene) noexcept
		: 
		OutlinerTreeItem(ETreeItemType::Scene),
		m_pScene{ pScene }
	{
	}

	//void OutlinerSceneTableData::SetAndPropagateVisibleState(bool visibleState) noexcept
	//{
	//	const bool isVisible = IsVisible();
	//
	//	if (isVisible == visibleState)
	//		return;
	//
	//	const auto& pSlice = GetConstSlice();
	//	const std::vector<std::shared_ptr<TableData>>& children = pSlice->GetData();
	//	for (auto& child : children)
	//		static_cast<OutlinerTableData*>(child.get())->SetAndPropagateVisibleState(visibleState);
	//}

	Scene* OutlinerSceneTreeItem::GetScene() noexcept
	{
		return m_pScene;
	}

	OutlinerFolderTreeItem::OutlinerFolderTreeItem(const char* name) noexcept
		:
		OutlinerTreeItem(ETreeItemType::Filter),
		m_Name{name}
	{
	}

	//void OutlinerFolderTableData::SetAndPropagateVisibleState(bool visibleState) noexcept
	//{
	//	const bool isVisible = IsVisible();
	//
	//	if (visibleState == isVisible)
	//		return;
	//
	//	const auto& pSlice = GetConstSlice();
	//	const std::vector<std::shared_ptr<TableData>>& children = pSlice->GetData();
	//
	//	for (auto& child : children)
	//		static_cast<OutlinerTableData*>(child.get())->SetAndPropagateVisibleState(visibleState);
	//}

}
