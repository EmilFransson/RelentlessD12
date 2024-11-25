#pragma once
#include "Callback/Broadcaster.h"
namespace Relentless
{
	class Tree;
	class TreeItem;

	class TreeInteraction
	{
	public:
		TreeInteraction() noexcept;
		virtual ~TreeInteraction() noexcept = default;

		Broadcaster<void(std::shared_ptr<TreeItem> pTreeItem, uint32_t column, bool doubleClicked)> OnItemClicked;
		Broadcaster<void(std::shared_ptr<TreeItem> pTreeItem, uint32_t column)> OnMouseReleasedOnItem;
		Broadcaster<void(std::shared_ptr<TreeItem> pTreeItem, uint32_t column)> OnItemHovered;
		Broadcaster<void(std::shared_ptr<TreeItem> pTreeItem, uint32_t column)> OnMouseEnterItemRow;
		Broadcaster<void(std::shared_ptr<TreeItem> pTreeItem)> OnMouseExitItemRow;

	protected:
		void OnBeginTree() noexcept;
		void OnEndTree() noexcept;
		virtual void OnClickedOnItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column, bool doubleClicked) noexcept;
		virtual void OnReleasedMouseOnItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept;
		virtual void OnHoveringItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept;
	private:
		friend class Tree;

		std::shared_ptr<TreeItem> m_pLastTreeItemClicked = nullptr;
		std::shared_ptr<TreeItem> m_pLastTreeItemHovered = nullptr;

		bool m_HoveredItemThisFrame = false;
	};
}