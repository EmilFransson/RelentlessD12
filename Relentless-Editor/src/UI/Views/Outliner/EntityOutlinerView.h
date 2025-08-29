#pragma once
#include <Relentless.h>
#include "../../../Core/EntityFilters.h"
#include "../../OutlinerTableRow.h"

namespace Relentless
{
	class Editor;
	enum class ESelectionState : uint8;

	class EntityOutlinerView : public IWidget<EntityOutlinerView>
	{
	public:
		EntityOutlinerView(Editor* pEditor) noexcept;
		virtual ~EntityOutlinerView() noexcept override;
	private:
		NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; } 

		NO_DISCARD Ref<OutlinerListItem> CreateEntityListItem(entity e) noexcept;
		NO_DISCARD Ref<OutlinerListItem> CreateEntityFilterListItem(EntityFilter* pFilter) noexcept;
		NO_DISCARD Ref<OutlinerListItem> CreateSceneListItem(Scene* pScene) noexcept;

		NO_DISCARD const String& GetItemName(const Ref<OutlinerListItem>& pItem) const noexcept;
		NO_DISCARD const String& GetRowName(const OutlinerTableRow* pRow) const noexcept;

		NO_DISCARD Ref<ContextMenu> OnContextMenuOpening(const Ref<OutlinerListItem>& item) noexcept;
		void OnCreateNewFilterButtonClicked() noexcept;

		void OnDeleteSelection() noexcept;
		void OnDuplicateSelection() noexcept;

		NO_DISCARD String OnDebugItemToString(const Ref<OutlinerListItem>& item) const noexcept;
		NO_DISCARD Ref<DragDropOperation> OnDragDetected(OutlinerTableRow* pRow) noexcept;
		NO_DISCARD bool OnDragEnter(OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) noexcept;
		NO_DISCARD bool OnDrop(OutlinerTableRow* pRow, OutlinerDragDropOperation& dragDropOp) noexcept;

		void OnEntityAttached(entity child, entity parent) noexcept;;
		void OnEntityCreated(entity newEntity) noexcept;
		void OnEntityDestroyed(entity destroyedEntity) noexcept;
		void OnEntityDetached(entity child, entity parent) noexcept;
		void OnEntityRemovedFromFilter(entity entity, EntityFilter* pFilter, bool filterToBeDestroyed) noexcept;
		void OnEntitySetToFilter(entity entity, EntityFilter* pFilter) noexcept;
		void OnEntityVisibilityChanged(entity e, bool visibilityState) noexcept;
		void OnExpandCollapseButtonClicked(Button* pButton, Ref<OutlinerListItem> pItem) noexcept;
		
		void OnFilterCreated(EntityFilter* pFilter) noexcept;
		void OnFilterReattached(EntityFilter* pChild, EntityFilter* pPreviousParent, EntityFilter* pNewParent) noexcept;
		void OnFocusChanged(bool focus) noexcept;

		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<OutlinerListItem>& item) noexcept;
		void OnGetChildren(const Ref<OutlinerListItem>& pParent, std::vector<Ref<OutlinerListItem>>& outChildren) noexcept;

		void OnMouseEnterButton(Button* pButton) noexcept;
		void OnMouseExitButton(Button* pButton) noexcept;

		void OnMouseEnterExpandCollapseButton(Button* pButton) noexcept;
		void OnMouseExitExpandCollapseButton(Button* pButton) noexcept;

		void OnMouseEnterRow(ITableRow* pTableRow) noexcept;
		void OnMouseExitRow(ITableRow* pTableRow) noexcept;

		void OnFilterDestroyed(EntityFilter* pFilter) noexcept;

		void OnPreSceneChanged(Scene* pScene) noexcept;

		void OnRender() noexcept override;
		void OnRenameSelection() noexcept;
		NO_DISCARD const std::vector<Ref<OutlinerListItem>>* OnRequestSource() noexcept;

		void OnSceneChanged(Scene* pScene) noexcept;
		void OnSearchTextChanged(const char* pText) noexcept;
		void OnSearchTextCommitted(const char* pText, ETextCommitType commitType) noexcept;
		void OnSelectionChanged(const Ref<OutlinerListItem>& item, ESelectionType selectionType) noexcept;
		void OnSelectionChangedExternally(entity e, ESelectionState selectionState) noexcept;
		
		void OnVisibilityButtonClicked(Button* pButton, Ref<OutlinerListItem> pItem) noexcept;
	private:
		std::vector<Ref<OutlinerListItem>> m_ListItems;
		Ref<TreeView<Ref<OutlinerListItem>>> m_pOutlinerTreeView = nullptr;

		std::unordered_map<entity, Ref<OutlinerListItem>> m_EntityToItemMap;
		std::unordered_map<EntityFilter*, Ref<OutlinerListItem>> m_FilterToItemMap;

		Ref<VerticalBox> m_pMainBox = nullptr;
		Ref<VerticalBox> m_pOutlinerListBox = nullptr;

		UniquePtr<TextFilterExpressionEvaluator> m_pFilter = nullptr;

		Editor* m_pEditor = nullptr;

		bool m_SuspendNotifications = false;
	};
}