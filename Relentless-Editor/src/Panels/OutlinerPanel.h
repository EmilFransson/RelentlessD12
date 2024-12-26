#pragma once
#include <Relentless.h>
#include "../UI/OutlinerTable.h"
#include "../UI/OutlinerTreeItem.h"

namespace Relentless
{
	class Editor;
	class EntityFilter;
	enum class ESelectionState : uint8_t;
	class Scene;

	enum class EColumnType : size_t { Visibility, Label, Type, Count };

	class OutlinerPanel
	{
	public:
		explicit OutlinerPanel(Editor* pEditor) noexcept;
		~OutlinerPanel() noexcept;
		void OnImGuiRender(const bool show) noexcept;
		void SelectAll() noexcept;
		void DeselectNonEntityItems() noexcept;

		[[nodiscard]] bool IsFocused() const noexcept;
	private:
		std::shared_ptr<OutlinerEntityTreeItem> CreateEntityTreeItem(entity e) noexcept;
		std::shared_ptr<OutlinerSceneTreeItem> CreateSceneTreeItem(Scene* pScene) noexcept;
		std::shared_ptr<OutlinerFilterTreeItem> CreateFilterTreeItem(EntityFilter* pFilter) noexcept;
		void AddToRoot(const std::shared_ptr<TreeItem>& pTreeItem) noexcept;
	
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

		void OnEntityFilterCreated(const std::string& path) noexcept;
		void OnEntityFilterDestroyed(const std::string& path) noexcept;
		void OnEntitySetToFilter(entity e, const std::string& path) noexcept;
		void OnEntityRemovedFromFilter(entity e, const std::string& path) noexcept;
		void OnEntityFilterReattached(const std::string& childFilterPathOld, const std::string& childFilterPathNew, const std::string& parentFilterPath) noexcept;

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
	private:
		Editor* m_pEditor = nullptr;
		Scene* m_pScene = nullptr;

		AssetHandle m_ShowEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_HideEntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_EntityTextureIconHandle = NULL_HANDLE;
		AssetHandle m_SceneTextureIconHandle = NULL_HANDLE;
		AssetHandle m_CheckTextureIconHandle = NULL_HANDLE;
		AssetHandle m_NotAllowedTextureIconHandle = NULL_HANDLE;
		AssetHandle m_EntityFilterOpenTextureIconHandle = NULL_HANDLE;
		AssetHandle m_EntityFilterClosedTextureIconHandle = NULL_HANDLE;

		AssetHandle m_DragDropTooltipIcon = NULL_HANDLE;

		std::shared_ptr<Outliner> m_pOutliner = nullptr;
		std::shared_ptr<TreeInteraction> m_pTreeInteraction = nullptr;
		std::shared_ptr<DragDropBehavior> m_pDragDropBehavior = nullptr;

		TreeItem* m_pReferenceSelection = nullptr;
		ETreeItemType m_LastSelectedType = ETreeItemType::None;
	
		std::unordered_map<entity, std::shared_ptr<OutlinerTreeItem>> m_EntityToTreeItemMap;
		mutable std::unordered_map<std::string, std::shared_ptr<OutlinerFilterTreeItem>> m_FilterPathToTreeItemMap;
		std::unordered_set<OutlinerTreeItem*> m_SelectedEntityFilters;

		bool m_DraggedOnValidTargetThisFrame = false;
		std::string m_DragDropTooltip{};

		bool m_SceneTreeItemSelected = false;

		bool m_SuspendNotifications = false;
	};
}