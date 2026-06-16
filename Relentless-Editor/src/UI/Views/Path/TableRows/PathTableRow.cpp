#include "PathTableRow.h"

#include "UI/Views/TreeView.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/EditableTextBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/WidgetSwitcher.h"

namespace Relentless
{
	PathTableRow::PathTableRow(const PathTableRowCreateInfo& aCreateInfo) noexcept
		: m_pOwningTreeView{ aCreateInfo.OwningTreeView }
	{
		Ref<HorizontalBox> pColumnBox = RLS_NEW HorizontalBox();
		pColumnBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		HorizontalBox* pLeftBox = pColumnBox->AddWidget(RLS_NEW HorizontalBox());
		pLeftBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
		pLeftBox->SetSize(Vector2(25.0f, -1.0f));

		m_pChevronButton = pLeftBox->AddWidget(RLS_NEW Button(aCreateInfo.IsExpanded ? ICON_FA_CHEVRON_DOWN : ICON_FA_CHEVRON_RIGHT, Vector2(25.0f, 30.0f)))
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Transparent)
			->SetBorderColor(Colors::Transparent)
			->SetTextColor(Colors::Gray)
			->SetAlpha(aCreateInfo.HasChildren ? 0.7f : 0.0f)
			->OnClicked(this, &PathTableRow::OnChevronButtonClicked);

		m_pChevronButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		m_pChevronButton->SetIsEnabled(aCreateInfo.HasChildren);

		HorizontalBox* pRightBox = pColumnBox->AddWidget(RLS_NEW HorizontalBox());
		pRightBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		pRightBox->AddWidget(RLS_NEW Label(aCreateInfo.IsExpanded && aCreateInfo.HasChildren ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER))
			->SetTextColor(Colors::FolderDefault)
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Transparent)
			->SetBorderColor(Colors::Transparent)
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pSwitcher = pRightBox->AddWidget(RLS_NEW WidgetSwitcher());
		m_pSwitcher->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pSwitcher->Add(RLS_NEW Label(aCreateInfo.DisplayName))
			->SetHighlightedSubstring(aCreateInfo.HighlightText)
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		m_pEditableTextBox = m_pSwitcher->Add(RLS_NEW EditableTextBox());
		m_pEditableTextBox->SetTextColor(Colors::TextDefault);
		m_pEditableTextBox->OnTextChanged(this, &PathTableRow::OnRenameTextChangedInternal);
		m_pEditableTextBox->OnTextCommitted(this, &PathTableRow::OnRenameTextCommittedInternal);
		m_pEditableTextBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		
		m_pSwitcher->SetActiveWidgetIndex(0);

		m_pExclamationLabel = pRightBox->AddWidget(RLS_NEW Label(ICON_FA_CIRCLE_EXCLAMATION));
		m_pExclamationLabel->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
		m_pExclamationLabel->SetIsVisible(false);
		m_pExclamationLabel->SetTextColor(Colors::Red);

		m_ColumnWidgets.push_back(pColumnBox);
	}

	const Color& PathTableRow::GetBackgroundColor() const noexcept
	{
		const bool isSelected = m_pOwningTreeView->IsItemSelected(m_pOwningTreeView->GetItemFromWidget(this));

		if (!isSelected && m_IsHovered)
			return Colors::RowHoverColorDefault;
		else if (isSelected && m_pOwningTreeView->IsFocused())
			return Colors::RowFocusedSelectionColorDefault;
		else if (isSelected && !m_pOwningTreeView->IsFocused())
			return Colors::RowUnfocusedSelectionColorDefault;
		else
		{
			const std::vector<Ref<PathListItem>> items = m_pOwningTreeView->GetDescendants(m_pOwningTreeView->GetItemFromWidget(this));
			if (std::ranges::any_of(items, [&](const Ref<PathListItem>& aItem)
				{
					return m_pOwningTreeView->IsItemSelected(aItem);
				}))
			{
				return Colors::RowAncestorToSelectedColorDefault;
			}
			else
				return Colors::Transparent;
		}
	}

	Ref<PathListItem> PathTableRow::GetItem() const noexcept
	{
		return m_pOwningTreeView->GetItemFromWidget(this);
	}

	uint32 PathTableRow::GetNumColumns() noexcept
	{
		return 1u;
	}

	void PathTableRow::HandleDragDrop() noexcept
	{
		UIModule& uiModule = ModuleManager::LoadModuleChecked<UIModule>();
		const bool chevronActiveAndHovered = m_pChevronButton->IsHovered() && m_pChevronButton->IsEnabled();

		if (SupportsDrag() && m_IsHovered && !chevronActiveAndHovered && !uiModule.HasActiveDragDrop() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceExtern | ImGuiDragDropFlags_SourceNoPreviewTooltip))
			{
				const Reply reply = OnDragDetected(m_Geometry, Mouse::CreatePointerInfo());
				if (reply.IsHandled())
					uiModule.SetActiveDragDropOperation(reply.GetDragDropOperation());

				ImGui::SetDragDropPayload("RLS_DRAGOP", nullptr, 0);
				ImGui::EndDragDropSource();
			}
		}

		if (uiModule.HasActiveDragDrop())
		{
			const ImVec2 mouse = ImGui::GetMousePos();
			const bool cursorOverRow = m_HasHoverRect && m_HoverRect.Contains(mouse);

			if (cursorOverRow)
			{
				const Ref<DragDropOperationBase>& pOp = uiModule.GetActiveDragDropOperation();

				uiModule.SetDragOverTarget(this, m_Geometry);

				const Reply reply = OnDragOver(m_Geometry, pOp);
				if (reply.IsHandled() && Mouse::IsButtonReleased(RLS_Button::Left))
				{
					OnDrop(m_Geometry, pOp);
					uiModule.ClearDragOverTarget();
					uiModule.ClearActiveDragDropOperation();
				}
			}
		}

		//if (ImGui::BeginDragDropTarget())
		//{
		//	if (uiModule.HasActiveDragDrop())
		//	{
		//		const Ref<DragDropOperationBase>& pOp = uiModule.GetActiveDragDropOperation();
		//		
		//		uiModule.SetDragOverTarget(this, m_Geometry);
		//
		//		//if (const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload("RLS_DRAGOP", ImGuiDragDropFlags_AcceptBeforeDelivery))
		//		//{
		//
		//		const Reply reply = OnDragOver(m_Geometry, pOp);
		//		if (reply.IsHandled())
		//		{
		//			if (Mouse::IsButtonReleased(RLS_Button::Left))
		//			{
		//				OnDrop(m_Geometry, pOp);
		//				uiModule.ClearDragOverTarget();
		//				uiModule.ClearActiveDragDropOperation();
		//			}
		//		}
		//		//}
		//	}
		//
		//	ImGui::EndDragDropTarget();
		//}
	}

	void PathTableRow::SetRenameFieldError(bool aIsError) noexcept
	{
		m_pEditableTextBox->SetTextColor(aIsError ? Colors::Red : Colors::TextDefault);
		m_pExclamationLabel->SetIsVisible(aIsError);
	}

	void PathTableRow::ShowEditableTextBox() noexcept
	{
		m_pSwitcher->SetActiveWidgetIndex(1u);
		
		Label* pLabel = static_cast<Label*>(m_pSwitcher->GetWidget(0).Get());
		EditableTextBox* pEditableTextBox = static_cast<EditableTextBox*>(m_pSwitcher->GetWidget(1).Get());
		pEditableTextBox->SetText(pLabel->GetText());
		pEditableTextBox->ForceKeyboardFocus();
	}

	void PathTableRow::ShowLabel() noexcept
	{
		m_pSwitcher->SetActiveWidgetIndex(0u);
	}

	void PathTableRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		if (!m_pOwningTreeView)
			return;

		const Ref<PathListItem>& item = m_pOwningTreeView->GetItemFromWidget(this);
		const ItemInfo& info = m_pOwningTreeView->GetItemInfo(item);

		for (uint32 i = 0u; i < info.Depth; ++i)
			ImGui::Indent();

		m_ColumnWidgets[aColumn]->AssignSize({ ImGui::GetContentRegionAvail().x, ReportSize().y });
		m_ColumnWidgets[aColumn]->Render();

		for (uint32 i = 0u; i < info.Depth; ++i)
			ImGui::Unindent();

		const bool chevronActiveAndHovered = m_pChevronButton->IsHovered() && m_pChevronButton->IsEnabled();
		if (m_IsHovered && !chevronActiveAndHovered)
		{
			const bool isSelected = m_pOwningTreeView->IsItemSelected(item);

			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnDoubleClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo(), this);
			else if (!isSelected && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
			else if (isSelected && !Keyboard::IsKeyDown(RLS_Key::LCtrl) && ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
			else if (isSelected && Keyboard::IsKeyDown(RLS_Key::LCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Right))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
		}
	}

	void PathTableRow::OnChevronButtonClicked() noexcept
	{
		const Ref<PathListItem>& pItem = m_pOwningTreeView->GetItemFromWidget(this);

		const ItemInfo& info = m_pOwningTreeView->GetItemInfo(pItem);
		m_pOwningTreeView->SetItemExpandedState(pItem, !info.IsExpanded);
		m_pOwningTreeView->RequestTreeRefresh();
	}

	void PathTableRow::OnRenameTextChangedInternal(const char* aText) noexcept
	{
		OnRenameTextChangedCallback.ExecuteIfSet(GetItem(), aText);
	}

	void PathTableRow::OnRenameTextCommittedInternal(const char* aText, ETextCommitType aTextCommitType) noexcept
	{
		OnRenameTextCommittedCallback.ExecuteIfSet(GetItem(), aText, aTextCommitType);
		m_pExclamationLabel->SetIsVisible(false);
	}

}