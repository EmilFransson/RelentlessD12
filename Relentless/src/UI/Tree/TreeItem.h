#pragma once
#include "Assets/AssetMeta.h"
#include "TreeTypes.h"

namespace Relentless
{
	class TreeItem
	{
	public:
		TreeItem(TreeItem* pParent = nullptr) noexcept;
		virtual ~TreeItem() noexcept = default;

		void SetBackgroundColor(const ImVec4& backgroundColor) noexcept;
		void SetColumnIcon(const AssetHandle& iconHandle, uint32_t column) noexcept;
		void SetIconTint(uint32_t column, const ImVec4& tint) noexcept;

		void ResetBackgroundColor() noexcept;

		[[nodiscard]] const std::string& GetColumnLabel(uint32_t columnIndex) const noexcept;
		[[nodiscard]] AssetHandle GetColumnIcon(uint32_t columnIndex) const noexcept;

		void AddChild(const std::shared_ptr<TreeItem>& pTreeItem) noexcept;
		void RemoveChild(const std::shared_ptr<TreeItem>& pTreeItem) noexcept;
		[[nodiscard]] bool HasChildren() const noexcept;
		[[nodiscard]] const std::vector<std::shared_ptr<TreeItem>>& GetChildren() const noexcept;

		void SetParent(TreeItem* pParent) noexcept;
		void RemoveParent() noexcept;
		[[nodiscard]] bool HasParent() const noexcept;
		[[nodiscard]] TreeItem* GetParent() const noexcept;

		void SetExpanded(bool expandState) noexcept;
		[[nodiscard]] bool IsExpanded() const noexcept;

		void SetData(const TreeItemData& data) noexcept;
		[[nodiscard]] const TreeItemData& GetData() const noexcept;
		
	private:
		bool m_IsExpanded = true;
	
		TreeItemData m_Data;
		TreeItem* m_pParent = nullptr;
		std::vector<std::shared_ptr<TreeItem>> m_Children;
	};
}
