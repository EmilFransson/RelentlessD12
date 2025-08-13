#include "ITableRow.h"

#include "UIManager.h"

namespace Relentless
{
	void ITableRow::OnRender() noexcept
	{
		ImGuiTable* pTable = ImGui::GetCurrentTable();
		if (!pTable)
			return;

		ImGui::TableNextRow();

		const float rowHeight = ImGui::GetFrameHeightWithSpacing();
		const float minX = pTable->Columns[0].MinX;
		const float maxX = pTable->Columns[pTable->ColumnsCount - 1].MaxX;
		const float fullWidth = maxX - minX;

		const ImVec2 rowStart(minX, ImGui::GetCursorScreenPos().y);
		const ImVec2 rowEnd(maxX, rowStart.y + rowHeight);

		for (uint32 column = 0u; column < GetNumColumns(); ++column)
		{
			ImGui::TableSetColumnIndex(column);
			OnRenderColumn(column);
		}

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

}