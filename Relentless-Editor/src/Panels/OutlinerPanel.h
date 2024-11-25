#pragma once
#include <Relentless.h>
#include "../UI/OutlinerTable.h"
#include "../UI/OutlinerTreeItem.h"

namespace Relentless
{
	class Editor;
	enum class ESelectionState : uint8_t;
	class Scene;

	enum class EColumnType : size_t { Visibility, Label, Type, Count };

	class OutlinerPanel
	{
	public:
		explicit OutlinerPanel(Editor* pEditor) noexcept;
		~OutlinerPanel() noexcept;
		void OnImGuiRender(const bool show) noexcept;

		void OnUpdate() noexcept;
	private:

		std::shared_ptr<OutlinerEntityTreeItem> CreateEntityTreeItem(entity e) noexcept;
	
		//Callbacks:
		void OnEditorSceneChanged(Scene* pScene) noexcept;
		void OnEntityCreated(entity e) noexcept;
		void OnEntityPreDestroyed(entity e) noexcept;
		void OnEntityAttached(entity child, entity parent) noexcept;
		void OnEntityDetached(entity child, entity parent) noexcept;
		void OnEntitySelectionChangedFromEditor(entity e, ESelectionState selectionState) noexcept;
		void OnEntityVisibilityChanged(entity e, bool visibilityState) noexcept;
		void OnOutlinerTreeFocusChanged(bool isFocused) noexcept;
		void OnTreeItemClicked(std::shared_ptr<TreeItem> pTreeItem, uint32_t column, bool doubleClicked) noexcept;
		void OnTreeItemHovered(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept;
		void OnMouseEnterTreeItemRow(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept;
		void OnMouseExitTreeItemRow(std::shared_ptr<TreeItem> pTreeItem) noexcept;
		void OnMouseReleasedOnTreeItem(std::shared_ptr<TreeItem> pTreeItem, uint32_t column) noexcept;
		bool OnDragOver(const Any& payload, const Any& target, std::string_view dropContext) noexcept;
		void OnDrop(const Any& payload, const Any& target, std::string_view dropContext) noexcept;

		void SetAndPropagateTreeItemVisibility(OutlinerTreeItem* pOutlinerTreeItem, bool visibilityState) noexcept;
		void SetReferenceSelection(OutlinerTreeItem* pReferenceSelection);

		void DetermineAndIssueSelection(OutlinerTreeItem* pTreeItem, uint32_t clickedColumn) noexcept;
		void SelectTreeItem(OutlinerTreeItem* pTreeItem) noexcept;
		void DeselectTreeItem(OutlinerTreeItem* pTreeItem) noexcept;
		void DeselectAllTreeItems() noexcept;

		[[nodiscard]] bool IsTreeItemSelected(TreeItem* pTreeItem) const noexcept;
		[[nodiscard]] bool IsTreeItemSelectableAtClickedColumn(TreeItem* pTreeItem, uint32_t column) const noexcept;
		[[nodiscard]] std::vector<std::shared_ptr<TreeItem>> GetAllSelectedTreeItems() const noexcept;

		[[nodiscard]] uint32_t GetNumSelected() const noexcept;

		bool IsAncestorToAnySelected(const std::shared_ptr<TreeItem>& treeItem, const std::vector<std::shared_ptr<TreeItem>>& selected) const noexcept;
	private:
		Editor* m_pEditor = nullptr;
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;

		std::shared_ptr<Outliner> m_pOutliner = nullptr;
		std::shared_ptr<TreeInteraction> m_pTreeInteraction = nullptr;
		std::shared_ptr<DragDropBehavior> m_pDragDropBehavior = nullptr;

		TreeItem* m_pReferenceSelection = nullptr;
		ETreeItemType m_LastSelectedType = ETreeItemType::None;
	
		std::unordered_map<entity, std::shared_ptr<OutlinerTreeItem>> m_EntityToTreeItemMap;

		bool m_DraggedOnValidTargetThisFrame = false;
		std::string m_DragDropTooltip{};
	};
}