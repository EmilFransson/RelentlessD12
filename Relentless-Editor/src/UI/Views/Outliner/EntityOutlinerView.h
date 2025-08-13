#pragma once
#include <Relentless.h>
#include "../../../Core/EntityFilters.h"
#include "../../OutlinerTableRow.h"

namespace Relentless
{
	class Editor;

	class EntityOutlinerView : public IWidget<EntityOutlinerView>
	{
	public:
		EntityOutlinerView(Editor* pEditor) noexcept;
		virtual ~EntityOutlinerView() noexcept override;
	private:
		NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; }

		NO_DISCARD Ref<ContextMenu> OnContextMenuOpening(const Ref<OutlinerListItem>& item) noexcept;

		void OnDelete() noexcept;

		NO_DISCARD String OnDebugItemToString(const Ref<OutlinerListItem>& item) const noexcept;
		void OnEntityCreated(entity newEntity) noexcept;
		void OnFocusChanged(bool focus) noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<OutlinerListItem>& item) noexcept;

		void OnMouseEnterButton(Button* pButton) noexcept;
		void OnMouseExitButton(Button* pButton) noexcept;

		void OnMouseEnterRow(ITableRow* pTableRow) noexcept;
		void OnMouseExitRow(ITableRow* pTableRow) noexcept;

		void OnRender() noexcept override;
		NO_DISCARD const std::vector<Ref<OutlinerListItem>>* OnRequestSource() const noexcept;

		void OnSearchTextChanged(const char* pText) noexcept;
		void OnSearchTextCommitted(const char* pText, ETextCommitType commitType) noexcept;
		void OnSelectionChanged(const Ref<OutlinerListItem>& item, ESelectionType selectionType) noexcept;
		
		void OnVisibilityButtonClicked(Button* pButton) noexcept;
	private:
		std::vector<Ref<OutlinerListItem>> m_ListItems;
		Ref<ListView<Ref<OutlinerListItem>>> m_pOutlinerListView = nullptr;

		std::unordered_set<entity> m_SelectedEntities;

		Ref<VerticalBox> m_pMainBox = nullptr;
		Ref<VerticalBox> m_pOutlinerListBox = nullptr;

		UniquePtr<TextFilterExpressionEvaluator> m_pFilter = nullptr;

		Editor* m_pEditor = nullptr;

		bool m_SuspendNotifications = false;
	};
}