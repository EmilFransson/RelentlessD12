#include "TreeInteraction.h"
#include "Tree.h"

namespace Relentless
{
	TreeInteraction::TreeInteraction() noexcept
	{
	}

	void TreeInteraction::OnBeginTree() noexcept
	{
		m_HoveredItemThisFrame = false;
	}

	void TreeInteraction::OnEndTree() noexcept
	{
		if (!m_HoveredItemThisFrame && m_pLastTreeItemHovered != nullptr)
		{
			OnMouseExitItemRow(m_pLastTreeItemHovered);
			m_pLastTreeItemHovered = nullptr;
		}
	}

	void TreeInteraction::OnClickedOnItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column, bool doubleClicked) noexcept
	{
		OnItemClicked(pTreeItem, column, doubleClicked);
		m_pLastTreeItemClicked = pTreeItem;
	}

	void TreeInteraction::OnReleasedMouseOnItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept
	{
		OnMouseReleasedOnItem(pTreeItem, column);
	}

	void TreeInteraction::OnHoveringItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept
	{
		if (m_pLastTreeItemHovered != nullptr && m_pLastTreeItemHovered != pTreeItem)
			OnMouseExitItemRow(m_pLastTreeItemHovered);
		if (m_pLastTreeItemHovered != pTreeItem)
			OnMouseEnterItemRow(pTreeItem, column);

		OnItemHovered(pTreeItem, column);
		m_pLastTreeItemHovered = pTreeItem;
	
		m_HoveredItemThisFrame = true;
	}
}

