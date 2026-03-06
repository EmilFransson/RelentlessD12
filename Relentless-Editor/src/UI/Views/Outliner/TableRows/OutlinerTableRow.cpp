#include "OutlinerTableRow.h"

#include "UI/Views/Outliner/EntityOutlinerView.h"

#include "UI/Widgets/Button.h"
#include "UI/Widgets/EditableTextBox.h"
#include "UI/Widgets/Label.h"
#include "UI/Widgets/WidgetSwitcher.h"

namespace Relentless
{
	OutlinerTableRow::OutlinerTableRow(const OutlinerTableRowCreateInfo& createInfo) noexcept
		: m_pOwningTreeView{ createInfo.pTreeView }
	{
		{
			Ref<HorizontalBox> pColumn0Box = new HorizontalBox();

			pColumn0Box->AddWidget(new Button(createInfo.IsVisible ? ICON_FA_EYE : ICON_FA_EYE_SLASH))
				->SetBackgroundColor(Colors::Transparent)
				->SetActiveColor(Colors::Transparent)
				->SetHoverColor(Colors::Transparent)
				->SetBorderColor(Colors::Transparent)
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetAlpha(0.7f)
				->SetTooltipText("Toggles the visibility of this item")
				->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center)
				->SetIsVisible((createInfo.IsVisible && !createInfo.IsSelected) ? false : true);

			m_ColumnWidgets.push_back(pColumn0Box);
		}
		
		{
			Ref<HorizontalBox> pColumn1Box = new HorizontalBox();
			pColumn1Box->SetMargin(FloatRect(0.0f, 0.0f, 0.0f, 0.0f));

			Button* pButton = pColumn1Box->AddWidget(new Button(createInfo.IsExpanded ? ICON_FA_CHEVRON_DOWN : ICON_FA_CHEVRON_RIGHT, Vector2(25.0f, 30.0f)))
				->SetBackgroundColor(Colors::Transparent)
				->SetActiveColor(Colors::Transparent)
				->SetHoverColor(Colors::Transparent)
				->SetBorderColor(Colors::Transparent)
				->SetTextColor(Colors::Gray)
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetAlpha(createInfo.HasChildren ? 0.7f : 0.0f);

			pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
			pButton->SetIsEnabled(createInfo.HasChildren);

			pColumn1Box->AddWidget(new Label(createInfo.Icon, ImGui::GetIO().Fonts->Fonts[2]))
				->SetTooltipText(createInfo.Name)
				->SetAlpha(0.8f)
				->SetTextColor(createInfo.IconColor)
				->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

			WidgetSwitcher* pSwitcher = pColumn1Box->AddWidget(new WidgetSwitcher());
			pSwitcher->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

			Label* pDisplayNameLabel = pSwitcher->Add(RLS_NEW Label(createInfo.Name, ImGui::GetIO().Fonts->Fonts[2]));
			pDisplayNameLabel->SetTooltipText(createInfo.Name);
			pDisplayNameLabel->SetHorizontalSizePolicy(ESizePolicy::Stretch);

			pSwitcher->Add(new EditableTextBox())
				->SetHorizontalSizePolicy(ESizePolicy::Stretch);

			pSwitcher->SetActiveWidgetIndex(0);

			m_ColumnWidgets.push_back(pColumn1Box);
		}

		{
			Ref<HorizontalBox> pColumn2Box = new HorizontalBox();

			pColumn2Box->AddWidget(new Label(createInfo.Type, ImGui::GetIO().Fonts->Fonts[2]))
				->SetTooltipText(createInfo.Type)
				->SetAlpha(0.7f);

			m_ColumnWidgets.push_back(pColumn2Box);
		}
	}

	Ref<IBaseWidget> OutlinerTableRow::GetWidget(uint8 column) noexcept
	{
		return m_ColumnWidgets[column];
	}

	void OutlinerTableRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		if (!m_pOwningTreeView)
			return;

		const Ref<OutlinerListItem>& item = m_pOwningTreeView->GetItemFromWidget(this);
		const ItemInfo& info = m_pOwningTreeView->GetItemInfo(item);
		m_Selected = m_pOwningTreeView->IsItemSelected(item);

		const ImVec2 currentPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ currentPos.x + m_Margins[aColumn].Left, currentPos.y + m_Margins[aColumn].Top });

		if (aColumn == 1)
		{
			for (uint32 i = 0u; i < info.Depth; ++i)
				ImGui::Indent();
		}
		
		m_ColumnWidgets[aColumn]->AssignSize({ ImGui::GetContentRegionAvail().x, ReportSize().y });
		m_ColumnWidgets[aColumn]->Render();

		if (aColumn == 1)
		{
			for (uint32 i = 0u; i < info.Depth; ++i)
				ImGui::Unindent();
		}

		//TODO: MOVE into parent -> ONCE, not EVERY ROW!
		if (m_Hovered && !m_ColumnWidgets[0]->IsHovered() && !(static_cast<HorizontalBox*>(m_ColumnWidgets[1].Get())->GetWidget<HorizontalBox>(0)->IsHovered()) && aColumn == 2)
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnDoubleClickedCallback.ExecuteIfSet();
			else if (!m_Selected && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
			else if (m_Selected && !Keyboard::IsKeyDown(RLS_Key::LCtrl) && ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
			else if (m_Selected && Keyboard::IsKeyDown(RLS_Key::LCtrl) && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
			else if (ImGui::IsMouseReleased(ImGuiMouseButton_::ImGuiMouseButton_Right))
				m_OnClickedCallback.ExecuteIfSet(Mouse::CreatePointerInfo());
		}
	}

	const Color& OutlinerTableRow::GetBackgroundColor() const noexcept
	{
		if (!m_Selected && m_Hovered)
			return Colors::RowHoverColorDefault;
		else if (m_Selected && m_pOwningTreeView->IsFocused())
			return Colors::RowFocusedSelectionColorDefault;
		else if (m_Selected && !m_pOwningTreeView->IsFocused())
			return Colors::RowUnfocusedSelectionColorDefault;
		else
		{
			const std::vector<Ref<OutlinerListItem>> items = m_pOwningTreeView->GetDescendants(m_pOwningTreeView->GetItemFromWidget(this));
			if (std::any_of(items.begin(), items.end(), [&](const Ref<OutlinerListItem>& pItem)
				{
					return m_pOwningTreeView->IsItemSelected(pItem);
				}))
			{
				return Colors::RowAncestorToSelectedColorDefault;
			}
			else
				return Colors::Transparent;

		}
	}

	EditableTextBox* OutlinerTableRow::GetEditableTextBox() const noexcept
	{
		return static_cast<EditableTextBox*>(GetWidgetSwitcher()->GetWidget(1).Get());
	}

	uint32 OutlinerTableRow::GetNumColumns() noexcept
	{
		return 3u;
	}

	WidgetSwitcher* OutlinerTableRow::GetWidgetSwitcher() const noexcept
	{
		return m_ColumnWidgets[1]->GetWidget<WidgetSwitcher>(2);
	}

	Button* OutlinerTableRow::GetExpandButton() const noexcept
	{
		return m_ColumnWidgets[1]->GetWidget<Button>(0);
	}

	Label* OutlinerTableRow::GetNameLabel() const noexcept
	{
		WidgetSwitcher* pSwitcher = GetWidgetSwitcher();
		return static_cast<Label*>(pSwitcher->GetWidget(0).Get());
	}

	Label* OutlinerTableRow::GetTypeLabel() const noexcept
	{
		return m_ColumnWidgets[2]->GetWidget<Label>(0);
	}

	Button* OutlinerTableRow::GetVisibilityButton() const noexcept
	{
		return m_ColumnWidgets[0]->GetWidget<Button>(0);
	}

	bool OutlinerTableRow::IsDragDropEligible() noexcept
	{
		return true;
	}

	Ref<DragDropOperation> OutlinerTableRow::OnDragDetected() noexcept
	{
		return m_OnDragDetectedCallback(this);
	}

	bool OutlinerTableRow::OnDragEnter(const Ref<DragDropOperation>& pDragDropOperation) noexcept
	{
		if (!pDragDropOperation->Is<OutlinerDragDropOperation>())
			return false;

		return m_OnDragEnterCallback(this, pDragDropOperation->As<OutlinerDragDropOperation>());
	}

	bool OutlinerTableRow::OnDragLeave(const Ref<DragDropOperation>& pDragDropOperation) noexcept
	{
		if (!pDragDropOperation->Is<OutlinerDragDropOperation>())
			return false;

		return true;
	}

	bool OutlinerTableRow::OnDrop(const Ref<DragDropOperation>& pDragDropOperation) noexcept
	{
		if (!pDragDropOperation->Is<OutlinerDragDropOperation>())
			return false;

		return  m_OnDropCallback(this, pDragDropOperation->As<OutlinerDragDropOperation>());
	}

}