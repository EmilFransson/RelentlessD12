#include "TreeItem.h"
#include "Tree.h"

namespace Relentless
{
	TreeItem::TreeItem(TreeItem* pParent) noexcept
		: m_pParent{ pParent }
	{
	}

	void TreeItem::SetBackgroundColor(const ImVec4& backgroundColor) noexcept
	{
		m_Data.BackgroundColor = backgroundColor;
		m_Data.UseDefaultItemColor = false;
	}

	void TreeItem::SetColumnIcon(const AssetHandle& iconHandle, uint32_t column) noexcept
	{
		m_Data.ColumnIcons[column].IconTextureHandle = iconHandle;
	}

	void TreeItem::SetIconTint(uint32_t column, const ImVec4& tint) noexcept
	{
		m_Data.ColumnIcons[column].Tint = tint;
	}

	void TreeItem::ResetBackgroundColor() noexcept
	{
		m_Data.UseDefaultItemColor = true;
	}

	const std::string& TreeItem::GetColumnLabel(uint32_t columnIndex) const noexcept
	{
		return m_Data.ColumnLabels[columnIndex];
	}

	AssetHandle TreeItem::GetColumnIcon(uint32_t columnIndex) const noexcept
	{
		return m_Data.ColumnIcons[columnIndex].IconTextureHandle;
	}

	void TreeItem::AddChild(const std::shared_ptr<TreeItem>& pTableData) noexcept
	{
		m_Children.push_back(pTableData);
		m_Children.back()->SetParent(this);
	}

	void TreeItem::RemoveChild(const std::shared_ptr<TreeItem>& pTreeItem) noexcept
	{
		std::erase_if(m_Children, [pTreeItem](const std::shared_ptr<TreeItem>& pChild)
			{
				return pChild == pTreeItem;
			});
	}

	bool TreeItem::HasChildren() const noexcept
	{
		return !m_Children.empty();
	}

	bool TreeItem::HasChild(const std::shared_ptr<TreeItem>& pTreeItem) const noexcept
	{
		return std::find_if(m_Children.begin(), m_Children.end(), [&](const std::shared_ptr<TreeItem>& pCurrentChild)
			{
				return pCurrentChild == pTreeItem;
			}) != m_Children.end();
	}

	const std::vector<std::shared_ptr<TreeItem>>& TreeItem::GetChildren() const noexcept
	{
		return m_Children;
	}

	std::vector<TreeItem*> TreeItem::GetAncestors() const noexcept
	{
		std::vector<TreeItem*> ancestors;

		TreeItem* pCurrent = m_pParent;
		while (pCurrent)
		{
			ancestors.push_back(pCurrent);
			pCurrent = pCurrent->GetParent();
		}

		return ancestors;
	}

	std::vector<TreeItem*> TreeItem::GetDescendants() noexcept
	{
		std::vector<TreeItem*> descendants;

		std::function<void(TreeItem* pTreeItem)> RecursivelyGetDescendants;

		RecursivelyGetDescendants = [&](TreeItem* pCurrentTreeItem)
		{
			descendants.push_back(pCurrentTreeItem);
			const std::vector<std::shared_ptr<TreeItem>>& children = pCurrentTreeItem->GetChildren();
			for (auto& child : children)
				RecursivelyGetDescendants(child.get());
		};

		for (auto& child : m_Children)
			RecursivelyGetDescendants(child.get());

		return descendants;
	}

	void TreeItem::SetParent(TreeItem* pParent) noexcept
	{
		m_pParent = pParent;
	}

	void TreeItem::RemoveParent() noexcept
	{
		m_pParent = nullptr;
	}

	bool TreeItem::HasParent() const noexcept
	{
		return m_pParent != nullptr;
	}

	TreeItem* TreeItem::GetParent() const noexcept
	{
		return m_pParent;
	}

	void TreeItem::SetExpanded(bool expandState) noexcept
	{
		m_IsExpanded = expandState;
	}

	bool TreeItem::IsExpanded() const noexcept
	{
		return m_IsExpanded;
	}

	void TreeItem::SetData(const TreeItemData& data) noexcept
	{
		m_Data = data;
	}

	const TreeItemData& TreeItem::GetData() const noexcept
	{
		return m_Data;
	}

}