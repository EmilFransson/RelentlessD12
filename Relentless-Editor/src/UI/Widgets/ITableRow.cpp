#include "ITableRow.h"
#include "Module/UIModule.h"

namespace Relentless
{
	void ITableRow::OnRender() noexcept
	{
		ImVec2 rowStart;
		ImVec2 rowEnd;
		const Vector2 size = ReportSize();

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

			const float minX = pTable->Columns[0].MinX;
			const float maxX = pTable->Columns[pTable->ColumnsCount - 1].MaxX;

			rowStart = ImVec2(minX, ImGui::GetCursorScreenPos().y);
			rowEnd = ImVec2(maxX, rowStart.y + size.y);

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

				const Color& bgCol = GetBackgroundColor();
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
	}

	void ITableRow::SetColumnWidget(uint8 aColumnIndex, const Ref<HorizontalBox>& aWidget) noexcept
	{
		RLS_ASSERT(GetNumColumns() > aColumnIndex, "[ITableRow::SetColumnWidget]: index out of bounds error.");

		if (m_ColumnWidgets.capacity() < GetNumColumns())
			m_ColumnWidgets.resize(GetNumColumns());

		m_ColumnWidgets[aColumnIndex] = aWidget;
	}

	void ITableRow::SetIndentation(uint32 aIndentation) noexcept
	{
		m_IndentationLevel = aIndentation;
	}

	Vector2 ITableRow::ReportSize() const noexcept
	{
		Vector2 size = Vector2::Zero;

		for (const auto& pWidget : m_ColumnWidgets)
		{
			size.x += pWidget ? pWidget->ReportSize().x : 0.0f;
			size.y = Math::Max(size.y, pWidget ? pWidget->ReportSize().y : 0.0f);
		}

		return size;
	}

}