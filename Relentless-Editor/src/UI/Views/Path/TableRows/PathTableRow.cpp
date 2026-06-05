#include "PathTableRow.h"

#include "UI/Views/TreeView.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/Label.h"

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
		pRightBox->SetTooltipText(aCreateInfo.Tooltip);

		pRightBox->AddWidget(RLS_NEW Label(aCreateInfo.IsExpanded && aCreateInfo.HasChildren ? ICON_FA_FOLDER_OPEN : ICON_FA_FOLDER))
			->SetTextColor(Colors::FolderDefault)
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Transparent)
			->SetBorderColor(Colors::Transparent)
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

		pRightBox->AddWidget(RLS_NEW Label(aCreateInfo.DisplayName))
			->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);

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

	uint32 PathTableRow::GetNumColumns() noexcept
	{
		return 1u;
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
}