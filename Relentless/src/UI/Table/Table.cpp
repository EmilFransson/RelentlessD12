#include "Table.h"
#include "TableData.h"
#include "TableDataSelection.h"
#include "TableDataSlice.h"
#include "UI/UI.h"
#include "Input/Keyboard.h"
#include "Assets/AssetManager.h"

namespace Relentless 
{
	namespace Table_private
	{
		[[nodiscard]] TableDataSelection::SelectionMode GetSelectionMode() noexcept
		{
			if (Keyboard::IsKeyPressed(RLS_KEY::LCtrl))
				return TableDataSelection::SelectionMode::Toggle;
			else if (Keyboard::IsKeyPressed(RLS_KEY::LShift))
				return TableDataSelection::SelectionMode::Range;
			else
				return TableDataSelection::SelectionMode::Single;
		}
	}

	Table::Table() noexcept
	{
		//Default selection context:
		m_pSelectionContext = std::make_shared<TableDataSelection>(this);
	}

	Table::~Table() noexcept
	{

	}

	void Table::SetEvenRowColor(const ImVec4& evenRowColor) noexcept
	{
		m_EvenRowColor = evenRowColor;
	}

	void Table::SetOddRowColor(const ImVec4& oddRowColor) noexcept
	{
		m_OddRowColor = oddRowColor;
	}

	void Table::SetRowHoverColor(const ImVec4& rowHoverColor) noexcept
	{
		m_RowHoverColor = rowHoverColor;
	}

	void Table::SetFreezeHeader(bool state) noexcept
	{
		m_FreezeHeader = state;
	}

	void Table::SetFlags(ImGuiTableFlags flags)
	{
		m_Flags = flags;
	}

	void Table::SetTableDataSelection(const std::shared_ptr<TableDataSelection>& tableDataSelection) noexcept
	{
		m_pSelectionContext = tableDataSelection;
	}

	void Table::AddEntry(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		m_Entries.push_back(pTableData);
	}

	void Table::AddColumn(const ColumnProperties& column) noexcept
	{
		m_Columns.push_back(column);
	}

	std::vector<std::shared_ptr<TableData>>& Table::GetRootEntries() noexcept
	{
		return m_Entries;
	}


	const std::shared_ptr<TableDataSelection>& Table::GetTableDataSelection() const noexcept
	{
		return m_pSelectionContext;
	}

	void Table::RemoveAll() noexcept
	{
		m_Entries.clear();
	}


	void Table::ClearAllSelections() noexcept
	{
		m_pSelectionContext->DeselectAll();
	}

	bool Table::IsFocused() const noexcept
	{
		return m_IsFocused;
	}

	bool Table::IsHovered() const noexcept
	{
		return ImGui::IsMouseHoveringRect(m_OuterRect.Min, m_OuterRect.Max);
	}

	bool Table::IsHeaderSpaceHovered() const noexcept
	{
		return ImGui::IsMouseHoveringRect(m_HeaderRect.Min, m_HeaderRect.Max);
	}

	bool Table::IsRowSpaceHovered() const noexcept
	{
		return IsHovered() && !IsHeaderSpaceHovered();
	}

	void Table::DrawHeaderCellContent(const TableIcon& tableIcon, const std::string& label, UI::Alignment alignment, int column) noexcept
	{
		const bool hasIcon = tableIcon.IconTextureHandle.IsValid();
		const bool hasText = !label.empty();

		const ImRect cellRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
		const ImVec2 cellSize = cellRect.GetSize();
		const ImVec2 cellPos = cellRect.Min;

		ImVec2 iconSize(0.0f, 0.0f);
		if (hasIcon)
		{
			float iconHeight = cellSize.y * tableIcon.SizeWeight.y;
			iconSize = ImVec2(iconHeight, iconHeight);
		}

		ImVec2 textSize(0, 0);
		if (hasText)
			textSize = ImGui::CalcTextSize(label.c_str());

		float spacing = 0.0f;
		if (hasIcon && hasText)
			spacing = 8.0f;

		const float totalContentWidth = iconSize.x + spacing + textSize.x;
		const float paddingX = ImGui::GetStyle().CellPadding.x;

		float contentStartX = 0.0f;

		switch (alignment)
		{
		case UI::Alignment::Left:
			contentStartX = cellPos.x + paddingX;
			break;
		case UI::Alignment::Center:
			contentStartX = cellPos.x + (cellSize.x - totalContentWidth) * 0.5f;
			break;
		case UI::Alignment::Right:
			contentStartX = cellPos.x + cellSize.x - totalContentWidth - paddingX;
			break;
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}

		const ImVec2 cursorPos = ImGui::GetCursorPos();

		float currentX = contentStartX;

		if (hasIcon)
		{
			const float iconPosY = cellPos.y + (cellSize.y - iconSize.y) * 0.5f;

			ImGui::SetCursorScreenPos(ImVec2(currentX, iconPosY));

			const std::shared_ptr<Texture2D> pTexture = AssetManager::Get<Texture2D>(tableIcon.IconTextureHandle);
			const ImVec2 imageSize(iconSize);

			ImGui::Image((ImTextureID)pTexture->GetSRVDescriptorHandle().GPUHandle.ptr, imageSize, ImVec2(0, 0), ImVec2(1, 1), tableIcon.Tint);

			currentX += iconSize.x + spacing;
		}

		if (hasText)
		{
			const float textPosY = cellPos.y + (cellSize.y - textSize.y) * 0.5f - 1;
			ImGui::SetCursorScreenPos(ImVec2(currentX, textPosY));

			ImGui::TextUnformatted(label.c_str());
		}

		ImGui::SetCursorPos(cursorPos);
	}

	void Table::OnBeginDraw() noexcept
	{
		m_TreeNodeToggled = false;

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBg, m_EvenRowColor);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBgAlt, m_OddRowColor);

		m_pSelectionContext->OnDrawBegin();

		m_OuterRect.Min = ImGui::GetCursorScreenPos();
	}

	void Table::Draw() noexcept
	{
		OnBeginDraw();

		if (ImGui::BeginTable(GetID(), m_Columns.size(), m_Flags))
		{
			m_IsFocused = ImGui::IsWindowFocused();

			SetupHeaderAndColumns();
			HandleColumnTooltips(); 

			const std::vector<TableDataRow> flattenedHierarchy = FlattenTree(); 

			for (uint32_t entryIndex = 0u; entryIndex < flattenedHierarchy.size(); ++entryIndex)
			{
				auto& tableData = flattenedHierarchy[entryIndex].Entry;

				ImGui::TableNextRow(ImGuiTableRowFlags_::ImGuiTableRowFlags_None, ImGui::GetFrameHeight());
				DetermineAndSetRowBackgroundColor(tableData);
				DrawRow(tableData, flattenedHierarchy[entryIndex].IndentationLevel);
				HandleRowInteraction(tableData);
			}

			ImGui::EndTable();
		}

		OnEndDraw();
	}

	void Table::OnEndDraw() noexcept
	{
		m_OuterRect.Max = ImGui::GetCursorScreenPos();
		m_OuterRect.Max.x += ImGui::GetContentRegionAvail().x;
		m_OuterRect.Max.y -= ImGui::GetStyle().ItemSpacing.y;

		ImGui::PopStyleColor(2);
		m_pSelectionContext->OnDrawEnd();
	}

	void Table::DrawRow(const std::shared_ptr<TableData>& tableData, uint32_t indentationLevel) noexcept
	{
		for (uint32_t columnIndex = 0u; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);
			DrawRowCell(tableData, columnIndex, indentationLevel);
		}
	}

	void Table::DrawRowCell(const std::shared_ptr<TableData>& tableData, int columnIndex, uint32_t indentationLevel)
	{
		const ColumnProperties& column = m_Columns[columnIndex];

		if (column.IsTreeNode)
		{
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen;

			if (tableData->HasChildren())
				nodeFlags |= (ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
			else
				nodeFlags |= ImGuiTreeNodeFlags_Leaf;

			ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));

			if (indentationLevel > 0)
				ImGui::Indent(indentationLevel * ImGui::GetTreeNodeToLabelSpacing() * 0.6f);
			
			void* nodeID = (uint32_t*)tableData.get() + columnIndex;
			const bool isExpanded = ImGui::TreeNodeEx(nodeID, nodeFlags, tableData->GetColumnString(columnIndex));
			m_TreeNodeToggled |= ImGui::IsItemToggledOpen();
			tableData->SetExpanded(isExpanded);

			ImGui::PopStyleColor(3);

			if (indentationLevel > 0)
				ImGui::Unindent(indentationLevel * ImGui::GetTreeNodeToLabelSpacing() * 0.6f);
		}
		else
			ImGui::Text(tableData->GetColumnString(columnIndex));
	}

	void Table::DetermineAndSetRowBackgroundColor(const std::shared_ptr<TableData>& tableData) noexcept
	{
		ImU32 targetColor = IM_COL32(0,0,0, 255);

		if (m_pSelectionContext->IsSelected(tableData))
			targetColor = m_IsFocused ? ImGui::GetColorU32(m_RowSelectedFocusedColor) : ImGui::GetColorU32(m_RowSelectedColor);
		else if (m_pSelectionContext->IsHovered(tableData))
			targetColor = ImGui::GetColorU32(m_RowHoverColor);
		else if (m_pSelectionContext->IsAncestorToAnySelected(tableData))
			targetColor = ImGui::GetColorU32(m_RowAncestorToSelectedColor);
		else
			return; // Use default color!

		for (uint32_t columnIndex = 0u; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, targetColor);
		}
	}

	void Table::SetupHeaderAndColumns() noexcept
	{
		if (m_FreezeHeader)
			ImGui::TableSetupScrollFreeze(0, 1);

		m_HeaderRect.Min = ImGui::GetCursorScreenPos();
		
		SetupColumns();
		ImGui::TableHeadersRow();

		for (int column = 0; column < m_Columns.size(); ++column)
		{
			ImGui::TableSetColumnIndex(column);
			DrawHeaderCellContent(m_Columns[column].HeaderIcon, m_Columns[column].Name, m_Columns[column].Alignment, column);
		}

		m_HeaderRect.Max = ImGui::GetCursorScreenPos();
		m_HeaderRect.Max.x += ImGui::GetContentRegionAvail().x;
	}

	void Table::SetupColumns() noexcept
	{
		for (int columnIndex = 0; columnIndex < m_Columns.size(); ++columnIndex)
		{
			const ColumnProperties& column = m_Columns[columnIndex];
			ImGui::TableSetupColumn("", column.Flags, column.DefaultWeight);
		}
	}

	void Table::HandleColumnTooltips() noexcept
	{
		for (int columnIndex = 0; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);

			const ImRect headerRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), columnIndex);
			if (ImGui::IsMouseHoveringRect(headerRect.Min, headerRect.Max))
			{
				if (const std::string& toolTip = m_Columns[columnIndex].HeaderTooltip; !toolTip.empty())
					UI::Utility::DrawTooltip(toolTip.c_str());
			}
		}
	}

	void Table::HandleRowInteraction(const std::shared_ptr<TableData>& tableData) noexcept
	{
		for (uint32_t columnIndex = 0u; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);

			const ImRect cellRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), columnIndex);
			const bool hoversCell = ImGui::IsMouseHoveringRect(cellRect.Min, cellRect.Max);

			if (hoversCell)
			{
				m_pSelectionContext->SetHovered(tableData);
				const char* toolTip = tableData->GetColumnTooltip(columnIndex);
				if (std::strcmp(toolTip, "") != 0)
					UI::Utility::DrawTooltip(toolTip);

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pSelectionContext->OnClickedOnRow(tableData, Table_private::GetSelectionMode(), columnIndex, true);
				}
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pSelectionContext->OnClickedOnRow(tableData, Table_private::GetSelectionMode(), columnIndex, false);
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pSelectionContext->OnReleasedOnRow(tableData, Table_private::GetSelectionMode());
				}
			}
		}
	}

	std::vector<Table::TableDataRow> Table::FlattenTree() const noexcept
	{
		std::vector<Table::TableDataRow> toReturn;
		uint32_t indentationLevel = 0u;

		std::function<void(const std::unique_ptr<TableDataSlice>&, uint32_t)> AddAllChildren;
		AddAllChildren = [&](const std::unique_ptr<TableDataSlice>& slice, uint32_t indentationLevel) 
			{
				const std::vector<std::shared_ptr<TableData>>& children = slice->GetData();
				for (auto& child : children)
				{
					toReturn.push_back({ child, indentationLevel });
					if (child->HasChildren() && child->IsExpanded())
						AddAllChildren(child->GetSlice(), indentationLevel + 1);
				}
			};

		for (uint32_t i = 0u; i < m_Entries.size(); ++i)
		{
			toReturn.push_back({ m_Entries[i], indentationLevel });
			if (m_Entries[i]->HasChildren() && m_Entries[i]->IsExpanded())
				AddAllChildren(m_Entries[i]->GetSlice(), indentationLevel + 1);
		}

		return toReturn;
	}
}
