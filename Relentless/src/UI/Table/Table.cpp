#include "Table.h"
#include "TableData.h"
#include "TableDataSelection.h"
#include "TableDataSlice.h"
#include "TableInteraction.h"
#include "TableStyling.h"
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
		m_pSelectionContext = std::make_shared<TableDataSelection>(this);
		m_pTableStyling = std::make_shared<TableStyling>(m_pSelectionContext.get());
	}

	Table::~Table() noexcept
	{

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

	void Table::SetTableStyling(const std::shared_ptr<TableStyling>& pTableStyling) noexcept
	{
		m_pTableStyling = pTableStyling;
	}

	void Table::SetTableInteraction(const std::shared_ptr<TableInteraction>& pTableInteraction) noexcept
	{
		m_pTableInteraction = pTableInteraction;
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

	bool Table::IsTreeNodeToggled() const noexcept
	{
		return m_TreeNodeToggled;
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
		const TableGeneralStyle& style = m_pTableStyling->GetGeneralStyle();

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBg, style.EvenRowColor);
		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBgAlt, style.OddRowColor);

		ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));

		m_pSelectionContext->OnDrawBegin();

		m_OuterRect.Min = ImGui::GetCursorScreenPos();
	}

	void Table::Draw() noexcept
	{
		OnBeginDraw();

		if (ImGui::BeginTable(GetID(), static_cast<int>(m_Columns.size()), m_Flags))
		{
			m_IsFocused = ImGui::IsWindowFocused();

			SetupHeaderAndColumns();
			HandleColumnTooltips(); 

			const std::vector<TableDataRow> flattenedHierarchy = FlattenTree(); 

			for (uint32_t entryIndex = 0u; entryIndex < flattenedHierarchy.size(); ++entryIndex)
			{
				auto& pTableData = flattenedHierarchy[entryIndex].Entry;

				ImGui::TableNextRow(ImGuiTableRowFlags_::ImGuiTableRowFlags_None, ImGui::GetFrameHeight());
				DetermineAndSetRowBackgroundColor(pTableData);
				DrawRow(pTableData, flattenedHierarchy[entryIndex].IndentationLevel);
				HandleRowInteraction(pTableData, flattenedHierarchy[entryIndex].IndentationLevel);
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

		ImGui::PopStyleColor(5);
		m_pSelectionContext->OnDrawEnd();
	}

	void Table::DrawRow(const std::shared_ptr<TableData>& pTableData, uint32_t indentationLevel) noexcept
	{
		for (uint32_t columnIndex = 0u; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);
			DrawRowCell(pTableData, columnIndex, indentationLevel);
		}
	}

	void Table::DrawRowCell(const std::shared_ptr<TableData>& pTableData, int columnIndex, uint32_t indentationLevel)
	{
		const ImRect cellRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), columnIndex);
		const ImVec2 cellSize = cellRect.GetSize();
		const ImVec2 cellPos = cellRect.Min;

		const ColumnProperties& column = m_Columns[columnIndex];
		const TableRowStyle rowStyle = m_pTableStyling->GetRowStyle(pTableData, columnIndex);
		const AssetHandle iconHandle = pTableData->GetColumnIcon(columnIndex);

		const char* label = pTableData->GetColumnString(columnIndex);

		const bool hasIcon = iconHandle.IsValid();
		const bool hasLabel = std::strcmp(label, "") != 0;

		if (column.IsTreeNode)
		{
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

			if (pTableData->HasChildren())
				nodeFlags |= (ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
			else
				nodeFlags |= ImGuiTreeNodeFlags_Leaf;

			if (indentationLevel > 0)
				ImGui::Indent(indentationLevel * ImGui::GetTreeNodeToLabelSpacing() * 0.6f);
			
			const void* nodeID = (uint32_t*)pTableData.get() + columnIndex;
			ImGui::SetNextItemOpen(pTableData->IsExpanded());
			const bool isExpanded = ImGui::TreeNodeEx(nodeID, nodeFlags, "");
			m_TreeNodeToggled |= ImGui::IsItemToggledOpen();
			pTableData->SetExpanded(isExpanded);

			ImGui::SameLine();
			if (hasIcon)
			{
				const float iconHeight = cellSize.y * 0.6f;
				const ImVec2 iconSize = ImVec2(iconHeight, iconHeight);
				const float iconPosY = cellPos.y + (cellSize.y - iconSize.y) * 0.5f;
				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, iconPosY));

				const std::shared_ptr<Texture2D> pIcon = AssetManager::Get<Texture2D>(iconHandle);
				ImGui::Image((ImTextureID)pIcon->GetSRVDescriptorHandle().GPUHandle.ptr, iconSize, ImVec2(0, 0), ImVec2(1, 1), rowStyle.IconTint);
				ImGui::SameLine();
			}

			if (hasLabel)
			{
				const float textHeight = UI::Utility::CalculateTextHeight(label) - GImGui->Style.CellPadding.y;
				const float offsetY = cellPos.y + (cellSize.y / 2.0f) - (textHeight / 2.0f);
				
				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, offsetY));
				ImGui::TextUnformatted(label);
			}

			if (indentationLevel > 0)
				ImGui::Unindent(indentationLevel * ImGui::GetTreeNodeToLabelSpacing() * 0.6f);
		}
		else
		{
			float contentWidth = 0.0f;
			if (hasLabel && hasIcon)
				contentWidth += rowStyle.Spacing == -1.0f ? GImGui->Style.ItemSpacing.x : rowStyle.Spacing;

			ImVec2 iconSize = ImVec2(0.0f, 0.0f);
			
			if (hasIcon)
			{
				float iconHeight = cellSize.y * rowStyle.IconWeight;
				iconSize = ImVec2(iconHeight, iconHeight);
				contentWidth += iconSize.x;
			}
			if (hasLabel)
				contentWidth += ImGui::CalcTextSize(label).x;

			const float offsetX = CalculateCellContentStartPosition(cellRect, rowStyle.Alignment, contentWidth);
			ImGui::SetCursorScreenPos(ImVec2(offsetX, ImGui::GetCursorScreenPos().y));

			if (hasIcon)
			{
				const float offsetY = cellPos.y + (cellSize.y / 2.0f) - (iconSize.y / 2.0f);
				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, offsetY));

				const std::shared_ptr<Texture2D> pIcon = AssetManager::Get<Texture2D>(iconHandle);
				ImGui::Image((ImTextureID)pIcon->GetSRVDescriptorHandle().GPUHandle.ptr, iconSize, ImVec2(0, 0), ImVec2(1, 1), rowStyle.IconTint);
				ImGui::SameLine(0.0f, rowStyle.Spacing);
			}

			if (hasLabel)
			{
				const float textHeight = UI::Utility::CalculateTextHeight(label) - GImGui->Style.CellPadding.y;
				const float offsetY = cellPos.y + (cellSize.y / 2.0f) - (textHeight / 2.0f);

				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, offsetY));
				ImGui::TextUnformatted(label);
			}
		}
	}

	void Table::DetermineAndSetRowBackgroundColor(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		const TableGeneralStyle& style = m_pTableStyling->GetGeneralStyle();

		ImU32 targetColor = IM_COL32(0,0,0, 255);

		if (m_pSelectionContext->IsSelected(pTableData))
			targetColor = m_IsFocused ? ImGui::GetColorU32(style.RowSelectedFocusedColor) : ImGui::GetColorU32(style.RowSelectedColor);
		else if (m_pSelectionContext->IsHovered(pTableData))
			targetColor = ImGui::GetColorU32(style.RowHoverColor);
		else if (m_pSelectionContext->IsAncestorToAnySelected(pTableData))
			targetColor = ImGui::GetColorU32(style.RowAncestorToSelectedColor);
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

	void Table::HandleRowInteraction(const std::shared_ptr<TableData>& pTableData, uint32_t indentationLevel) noexcept
	{
		for (uint32_t columnIndex = 0u; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);

			const ImRect cellRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), columnIndex);
			const ImVec2 cellSize = cellRect.GetSize();
			const ImVec2 cellPos = cellRect.Min;
			const bool hoversCell = ImGui::IsMouseHoveringRect(cellRect.Min, cellRect.Max);
			
			if (hoversCell)
			{
				m_pSelectionContext->SetHovered(pTableData, columnIndex);
				const char* toolTip = pTableData->GetColumnTooltip(columnIndex);
				if (std::strcmp(toolTip, "") != 0)
					UI::Utility::DrawTooltip(toolTip);

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pSelectionContext->OnClickedOnRow(pTableData, Table_private::GetSelectionMode(), columnIndex, true);
				}
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pSelectionContext->OnClickedOnRow(pTableData, Table_private::GetSelectionMode(), columnIndex, false);
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pSelectionContext->OnReleasedOnRow(pTableData, Table_private::GetSelectionMode());
				}
			}

			auto&& GetNonTreeNodeArrowArea = [&]() -> std::vector<ImRect>
			{
				std::vector<ImRect> toReturn;
				ImVec2 min = cellRect.Min;

				if (!(pTableData->HasChildren() && m_Columns[columnIndex].IsTreeNode))
				{
					ImVec2 max = cellRect.Max;
					toReturn.push_back(ImRect(min, max));
					return toReturn;
				}

				min.x += (ImGui::GetTreeNodeToLabelSpacing() * indentationLevel * 0.6f);
				min.x += GImGui->Style.ItemSpacing.x;

				const ImVec2 max = ImVec2(min.x, cellRect.Max.y);

				toReturn.push_back(ImRect(cellRect.Min, max));

				min.x += GImGui->Style.FramePadding.x;
				min.x += ImGui::GetFontSize();
				
				toReturn.push_back(ImRect(min, cellRect.Max));
			};

			// Set cursor to the top-left of the cell
			ImGui::SetCursorScreenPos(ImVec2(cellRect.Min.x, cellPos.y));

			const std::vector<ImRect> nonArrowAreas = GetNonTreeNodeArrowArea();
			for (uint32_t i = 0u; i < nonArrowAreas.size(); ++i)
			{
				const ImRect& area = nonArrowAreas[i];

				// Create a unique ID for the invisible button
				char buttonID[64];
				snprintf(buttonID, sizeof(buttonID), "##cell_%p_%p", pTableData.get(), columnIndex + pTableData.get());
				
				const float paddingY = ImGui::GetStyle().FramePadding.y;
				float adjustedHeight = cellSize.y - paddingY;
				if (adjustedHeight < 0.0f)
					adjustedHeight = 0.0f;
				
				// Create an invisible button that covers the non arrow cell parts:
				ImGui::PushID(buttonID);
				ImGui::InvisibleButton("", ImVec2(area.Max.x - area.Min.x, adjustedHeight));
				ImGui::SetItemAllowOverlap();
				ImGui::PopID();

				if (i + 1 < nonArrowAreas.size())
					ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x + (nonArrowAreas[i+1].Min.x - area.Max.x) + ((ImGui::GetTreeNodeToLabelSpacing() * indentationLevel * 0.6f)), cellPos.y));
			}

			if (!m_TreeNodeToggled && m_pTableInteraction && m_pTableInteraction->IsDragDropEnabled())
			{
				if (m_pTableInteraction->IsDraggable(pTableData, columnIndex))
				{
					if (ImGui::BeginDragDropSource())
					{
						const TableInteraction::PayloadInfo payloadInfo = m_pTableInteraction->GetPayloadInfo(m_pSelectionContext->GetSelected());
						ImGui::SetDragDropPayload(payloadInfo.ID, payloadInfo.Data, payloadInfo.Size, ImGuiCond_Once);
						if (payloadInfo.TooltipLabel)
							ImGui::Text(payloadInfo.TooltipLabel);

						ImGui::EndDragDropSource();
					}
				}
			}
			
		}
	}

	float Table::CalculateCellContentStartPosition(ImRect cellRect, UI::Alignment alignment, float contentWidth) const noexcept
	{
		const ImVec2 cellSize = cellRect.GetSize();
		const ImVec2 cellPos = cellRect.Min;
		const float paddingX = ImGui::GetStyle().CellPadding.x;

		float contentStartX = 0.0f;

		switch (alignment)
		{
		case UI::Alignment::Left:
			contentStartX = cellPos.x + paddingX;
			break;
		case UI::Alignment::Center:
			contentStartX = cellPos.x + (cellSize.x - contentWidth) * 0.5f;
			break;
		case UI::Alignment::Right:
			contentStartX = cellPos.x + cellSize.x - contentWidth - paddingX;
			break;
		default:
			RLS_ASSERT(false, "Unreachable.");
			break;
		}

		return contentStartX;
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
