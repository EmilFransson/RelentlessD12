#include "OutlinerTableRow.h"

#include "DragDrop/OutlinerDragDropOperation.h"

namespace Relentless
{
	OutlinerTableRow::OutlinerTableRow(ListView<Ref<OutlinerListItem>>* pListView) noexcept
		: m_pOwningListView{ pListView }
	{
		m_pDragDropTooltip = new Tooltip();
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
		if (!m_pOwningListView)
			return;

		const Ref<OutlinerListItem>& item = m_pOwningListView->GetItemFromWidget(this);
		m_Selected = m_pOwningListView->IsItemSelected(item);

		const ImVec2 currentPos = ImGui::GetCursorPos();
		ImGui::SetCursorPos({ currentPos.x + m_Margins[column].Left, currentPos.y + m_Margins[column].Top });

		m_ColumnWidgets[column]->Render();

		if (m_Hovered && !m_ColumnWidgets[0]->IsHovered())
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
		else if (!m_Selected && !m_Hovered)
			return Colors::Transparent;
		else if (m_Selected && m_pOwningListView->IsFocused())
			return Colors::RowFocusedSelectionColorDefault;
		else
			return Colors::RowUnfocusedSelectionColorDefault;
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
		Ref<OutlinerDragDropOperation> pEntityOp = new OutlinerDragDropOperation();
		pEntityOp->SetTooltipText(ICON_FA_BAN "  Cannot attach entity to self");

		return pEntityOp;
	}

	bool OutlinerTableRow::OnDragEnter(const Ref<DragDropOperation>& pDragDropOperation) noexcept
	{
		if (!pDragDropOperation->Is<OutlinerDragDropOperation>())
			return false;

		//Perhaps check payload or something else...
		RLS_CORE_INFO("ENTERED!");

		return true;
	}

	bool OutlinerTableRow::OnDragLeave(const Ref<DragDropOperation>& pDragDropOperation) noexcept
	{
		if (!pDragDropOperation->Is<OutlinerDragDropOperation>())
			return false;

		RLS_CORE_INFO("LEFT!");

		return true;
	}

	bool OutlinerTableRow::OnDrop(const Ref<DragDropOperation>& pDragDropOperation) noexcept
	{
		if (!pDragDropOperation->Is<OutlinerDragDropOperation>())
			return false;

		RLS_CORE_INFO("DROPPED!");

		//Do stuff... Perhaps callback...

		return true;
	}

}