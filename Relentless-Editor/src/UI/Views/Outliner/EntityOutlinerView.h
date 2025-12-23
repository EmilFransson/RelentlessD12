#pragma once
#include <Relentless.h>
#include "EntityOutlinerPolicies.h"

#include "../../../Core/EntityFolders.h"
#include "../../OutlinerTableRow.h"

namespace Relentless
{
	class Editor;
	enum class ESelectionState : uint8;

	class EntityOutlinerView : public IWidget<EntityOutlinerView>
	{
	public:
		EntityOutlinerView(std::weak_ptr<Editor> aEditor) noexcept;
		virtual ~EntityOutlinerView() noexcept override;

		void DuplicateSelection() noexcept;

		void OnDeleteSelection() noexcept;
		void OnRenameSelection() noexcept;
	private:
		NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; } 

		NO_DISCARD Ref<OutlinerListItem> CreateEntityListItem(entity aEntity) noexcept;
		NO_DISCARD Ref<OutlinerListItem> CreateEntityFolderListItem(EntityFolder* apFolder) noexcept;
		NO_DISCARD Ref<OutlinerListItem> CreateSceneListItem(Scene* pScene) noexcept;

		void DeselectAllFolders() noexcept;

		void ExecuteMovePlan(const EntityOutlinerPolicies::MovePlan& aMovePlan, Scene& aScene, const OutlinerPayload& targetPayload) noexcept;

		NO_DISCARD const String& GetItemName(const Ref<OutlinerListItem>& pItem) const noexcept;
		NO_DISCARD const String& GetRowName(const OutlinerTableRow* pRow) const noexcept;
		NO_DISCARD const Ref<OutlinerListItem>& GetEntityItem(entity aEntity) const noexcept;
		NO_DISCARD const Ref<OutlinerListItem>& GetFolderItem(EntityFolder* pAFolder) const noexcept;
		NO_DISCARD Ref<OutlinerListItem> GetRootSceneItem() const noexcept;

		NO_DISCARD Ref<ContextMenu> OnContextMenuOpening(const Ref<OutlinerListItem>& aItem) noexcept;
		void OnCreateNewFolderButtonClicked() noexcept;

		void OnDuplicateSelection() noexcept;

		NO_DISCARD String OnDebugItemToString(const Ref<OutlinerListItem>& apItem) const noexcept;
		NO_DISCARD Ref<DragDropOperation> OnDragDetected(OutlinerTableRow* apRow) noexcept;
		NO_DISCARD bool OnDragEnter(OutlinerTableRow* apRow, OutlinerDragDropOperation& aDragDropOp) noexcept;
		NO_DISCARD bool OnDrop(OutlinerTableRow* apRow, OutlinerDragDropOperation& aDragDropOp) noexcept;

		NO_DISCARD std::vector<EntityFolder*> MergeFoldersByLabel(const std::vector<EntityFolder*>& someFolders) noexcept;

		void OnEntityAttached(entity aChildEntity, entity aParentEntity) noexcept;
		void OnEntityCreated(entity aNewEntity) noexcept;
		void OnEntityDestroyed(entity aDestroyedEntity) noexcept;
		void OnEntityPreDestroyed(entity aDestroyedEntity) noexcept;
		void OnEntityDetached(entity aChildEntity, entity aParentEntity) noexcept;
		void OnEntityRemovedFromFolder(entity aEntity, const Folder& aFolder) noexcept;
		void OnEntityAttachedToFolder(entity aEntity, const Folder& aFolder) noexcept;
		void OnEntityVisibilityChanged(entity aEntity, bool aVisibilityState) noexcept;
		void OnExpandCollapseButtonClicked(Button* apButton, Ref<OutlinerListItem> apItem) noexcept;
		
		void OnFolderCreated(EntityFolder* apFolder) noexcept;
		void OnFolderMoved(EntityFolder* apChildFolder, EntityFolder* paOldParentFolder, const String& aOldPath, const String& aNewPath) noexcept;
		void OnFocusChanged(bool aFocus) noexcept;

		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<OutlinerListItem>& apItem) noexcept;
		void OnGetChildren(const Ref<OutlinerListItem>& apParent, std::vector<Ref<OutlinerListItem>>& outChildren) noexcept;

		void OnItemScrolledIntoView(const Ref<OutlinerListItem>& aItem) noexcept;

		void OnMouseEnterVisibilityButton(Button* pButton) noexcept;
		void OnMouseExitVisibilityButton(Button* pButton, OutlinerListItem* pItem) noexcept;

		void OnMouseEnterExpandCollapseButton(Button* apButton) noexcept;
		void OnMouseExitExpandCollapseButton(Button* apButton) noexcept;

		void OnMouseEnterRow(ITableRow* apTableRow) noexcept;
		void OnMouseExitRow(ITableRow* apTableRow) noexcept;

		void OnOutlinerTreeRefreshed() noexcept;

		void OnEntityFolderDeleted(EntityFolder* apFolder) noexcept;

		void OnPreSceneChanged(Scene* pScene) noexcept;

		void OnRender() noexcept override;
		NO_DISCARD const std::vector<Ref<OutlinerListItem>>* OnRequestSource() noexcept;
		void OnRowDoubleClicked(const Ref<OutlinerListItem>& apItem) noexcept;

		void OnSceneChanged(Scene* apScene) noexcept;
		void OnSearchTextChanged(const char* aText) noexcept;
		void OnSearchTextCommitted(const char* aText, ETextCommitType aCommitType) noexcept;
		void OnSelectionChanged(const Ref<OutlinerListItem>& apItem, ESelectionType aSelectionType) noexcept;
		void OnSelectionChangedExternally(entity aEntity, ESelectionState aSelectionState) noexcept;
		
		void OnVisibilityButtonClicked(Button* apButton, Ref<OutlinerListItem> apItem) noexcept;

		void RecreateItemHierarchy() noexcept;
		NO_DISCARD bool RenameFolder(EntityFolder* aFolder, const String& aName) noexcept;
		NO_DISCARD bool RenameEntity(entity aEntity, const String& aName) noexcept;

		void ToggleVisibilityForItem(const Ref<OutlinerListItem>& pAOutlinerListItem, bool aToVisible) noexcept;

		void UpdateEntityInfoBorder() noexcept;
	private:
		friend class EntityFoldersManager;

		std::vector<Ref<OutlinerListItem>> m_ListItems;
		Ref<TreeView<Ref<OutlinerListItem>>> m_pOutlinerTreeView = nullptr;

		std::unordered_map<UUID, Ref<OutlinerListItem>> m_ItemMap;

		std::unordered_map<UUID, Ref<OutlinerListItem>> m_EntityToItemMap;
		std::unordered_map<UUID, Ref<OutlinerListItem>> m_FolderToItemMap;
		std::unordered_set<UUID> m_SelectedFolders;

		Ref<VerticalBoxEx> m_pMainBox = nullptr;
		Ref<HorizontalBoxEx> m_pOutlinerListBox = nullptr;

		UniquePtr<TextFilterExpressionEvaluator> m_pFilter = nullptr;
		UniquePtr<EntityOutlinerPolicies> m_pPolicies = nullptr;

		Ref<OutlinerListItem> m_pItemToScrollIntoView = nullptr;
		Ref<OutlinerListItem> m_pFolderToRenameWhenScrolledIntoView = nullptr;

		std::weak_ptr<Editor> m_pEditor;

		bool m_SuspendNotifications = false;
		bool m_SceneItemSelected = false;
	};
}