#include "OutlinerTableRow.h"

#include "Views/Outliner/EntityOutlinerView.h"

namespace Relentless
{
	OutlinerTableRow::OutlinerTableRow(TreeView<Ref<OutlinerListItem>>* pTreeView) noexcept
		: m_pOwningTreeView{ pTreeView }
	{
	}

	float OutlinerTableRow::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	Ref<IBaseWidget> OutlinerTableRow::GetWidget(uint8 column) noexcept
	{
		return m_ColumnWidgets[column];
	}

	void OutlinerTableRow::OnRenderColumn(uint32 column) noexcept
	{
		if (!m_pOwningTreeView)
			return;

		const Ref<OutlinerListItem>& item = m_pOwningTreeView->GetItemFromWidget(this);
		const ItemInfo& info = m_pOwningTreeView->GetItemInfo(item);
		m_Selected = m_pOwningTreeView->IsItemSelected(item);

		const ImVec2 currentPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ currentPos.x + m_Margins[column].Left, currentPos.y + m_Margins[column].Top });

		if (column == 1)
		{
			for (uint32 i = 0u; i < info.Depth; ++i)
				ImGui::Indent();
		}
		
		m_ColumnWidgets[column]->Render();

		if (column == 1)
		{
			for (uint32 i = 0u; i < info.Depth; ++i)
				ImGui::Unindent();
		}

		//TODO: MOVE into parent -> ONCE, not EVERY ROW!
		if (m_Hovered && !m_ColumnWidgets[0]->IsHovered() && !(static_cast<HorizontalBox*>(m_ColumnWidgets[1].Get())->GetChild(0)->IsHovered()) && column == 2)
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

	uint32 OutlinerTableRow::GetNumColumns() noexcept
	{
		return 3u;
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