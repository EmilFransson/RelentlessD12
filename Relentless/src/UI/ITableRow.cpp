#include "ITableRow.h"

#include "UIManager.h"

namespace Relentless
{
	void ITableRow::OnRender() noexcept
	{
		ImVec2 rowStart;
		ImVec2 rowEnd;
		float fullWidth;
		float rowHeight;

		if (!m_Tiled)
		{
			ImGuiTable* pTable = ImGui::GetCurrentTable();
			if (!pTable)
				return;

			float maxHeight = 0.0f;
			for (uint32 column = 0u; column < m_ColumnWidgets.size(); ++column)
			{
				const float height = m_ColumnWidgets[column]->ReportSize().y;
				maxHeight = Math::Max(maxHeight, height);
			}

			ImGui::TableNextRow(0, maxHeight);

			rowHeight = ImGui::GetFrameHeightWithSpacing();
			const float minX = pTable->Columns[0].MinX;
			const float maxX = pTable->Columns[pTable->ColumnsCount - 1].MaxX;
			fullWidth = maxX - minX;

			rowStart = ImVec2(minX, ImGui::GetCursorScreenPos().y);
			rowEnd = ImVec2(maxX, rowStart.y + rowHeight);

			for (uint32 column = 0u; column < GetNumColumns(); ++column)
			{
				ImGui::TableSetColumnIndex(column);
				OnRenderColumn(column);
			}
		}
		else
		{
			for (uint32 column = 0u; column < GetNumColumns(); ++column)
				OnRenderColumn(column);
		}
		
		{
			if (!m_CustomHoverLogic)
			{
				ImGui::TablePushBackgroundChannel();

				Color bgCol = GetBackgroundColor();
				ImGui::GetWindowDrawList()->AddRectFilled(rowStart, rowEnd, ImGui::ColorConvertFloat4ToU32(ImVec4(bgCol.x, bgCol.y, bgCol.z, bgCol.w)));

				const bool isHovering = ImGui::IsMouseHoveringRect(rowStart, rowEnd);
				if (!m_Hovered && isHovering)
				{
					m_Hovered = true;
					m_OnMouseEnterCallback.ExecuteIfSet(this);
				}
				else if (m_Hovered && !isHovering)
				{
					m_Hovered = false;
					m_OnMouseExitCallback.ExecuteIfSet(this);
				}

				ImGui::TablePopBackgroundChannel();
			}
		}

		{
			if (IsDragDropEligible())
			{
				ImGui::PushID((const void*)this);
				ImGui::TablePushBackgroundChannel();
				
				ImGui::SetCursorScreenPos(rowStart);
				ImGui::InvisibleButton("##RowDragHandle", ImVec2(fullWidth, rowHeight));
				const uint64 dragDropAreaID = ImGui::GetItemID();

				UIManager& uiMgr = UIManager::Get();
				uiMgr.BeginDragDropSource([this]() { return OnDragDetected(); });
				uiMgr.BeginDragDropTarget(
					dragDropAreaID, 
					[this](const Ref<DragDropOperation>& pDragDropOp) { return OnDragEnter(pDragDropOp); }, 
					[this](const Ref<DragDropOperation>& pDragDropOp) { return OnDragLeave(pDragDropOp); },
					[this](const Ref<DragDropOperation>& pDragDropOp) { return OnDrop(pDragDropOp); });

				ImGui::TablePopBackgroundChannel();
				ImGui::PopID();
			}
		}
	}

	void ITableRow::SetColumnWidget(uint8 aColumnIndex, const Ref<HorizontalBoxEx>& aWidget) noexcept
	{
		RLS_ASSERT(GetNumColumns() > aColumnIndex, "[ITableRow::SetColumnWidget]: index out of bounds error.");

		if (m_ColumnWidgets.capacity() < GetNumColumns())
			m_ColumnWidgets.resize(GetNumColumns());

		m_ColumnWidgets[aColumnIndex] = aWidget;
	}

}