#include "Tree.h"

#include "Graphics/RHI/ResourceViews.h"
#include "UI/DragDropBehavior.h"
#include "TreeItem.h"
#include "TreeInteraction.h"
#include "TreeStyle.h"
#include "UI/UI.h"
#include "Input/Keyboard.h"
#include "Assets/AssetManager.h"

namespace Relentless 
{
	Tree::Tree(const char* id, const std::shared_ptr<TreeDataView>& dataView) noexcept
		: m_ID{id}
	{
		m_pDataView = dataView;

		if (!m_pDataView->pTreeInteraction)
			m_pDataView->pTreeInteraction = std::make_shared<TreeInteraction>();

		if (!m_pDataView->pTreeStyle)
			m_pDataView->pTreeStyle = std::make_shared<TreeStyle>();
	}

	Tree::~Tree() noexcept
	{

	}

	void Tree::SetFreezeHeader(bool state) noexcept
	{
		m_FreezeHeader = state;
	}

	void Tree::SetFlags(ImGuiTableFlags flags)
	{
		m_Flags = flags;
	}

	void Tree::AddEntry(const std::shared_ptr<TreeItem>& pTableData) noexcept
	{
		m_Entries.push_back(pTableData);
	}

	void Tree::RemoveEntry(const std::shared_ptr<TreeItem>& pTreeItem) noexcept
	{
		std::erase_if(m_Entries, [pTreeItem](const std::shared_ptr<TreeItem>& pEntry)
			{
				return pEntry == pTreeItem;
			});
	}

	void Tree::AddColumn(const ColumnProperties& column) noexcept
	{
		m_Columns.push_back(column);
	}

	std::vector<std::shared_ptr<TreeItem>>& Tree::GetRootEntries() noexcept
	{
		return m_Entries;
	}

	const std::shared_ptr<TableDataSelection>& Tree::GetTableDataSelection() const noexcept
	{
		return m_pSelectionContext;
	}

	void Tree::RemoveAll() noexcept
	{
		m_Entries.clear();
	}

	bool Tree::IsFocused() const noexcept
	{
		return m_IsFocused;
	}

	bool Tree::IsHovered() const noexcept
	{
		return ImGui::IsMouseHoveringRect(m_OuterRect.Min, m_OuterRect.Max);
	}

	bool Tree::IsHeaderSpaceHovered() const noexcept
	{
		return ImGui::IsMouseHoveringRect(m_HeaderRect.Min, m_HeaderRect.Max);
	}

	bool Tree::IsRowSpaceHovered() const noexcept
	{
		return IsHovered() && !IsHeaderSpaceHovered();
	}

	bool Tree::IsTreeNodeToggled() const noexcept
	{
		return m_TreeNodeToggled;
	}

	uint32 Tree::GetNumColumns() const noexcept
	{
		return static_cast<uint32>(m_Columns.size());
	}

	std::vector<std::shared_ptr<TreeItem>> Tree::GetDescendants(const std::shared_ptr<TreeItem>& pTreeItem) const noexcept
	{
		std::vector<std::shared_ptr<TreeItem>> descendants;

		std::function<void(const std::shared_ptr<TreeItem>& pTreeItem)> RecursivelyGetChildren;

		RecursivelyGetChildren = [&](const std::shared_ptr<TreeItem>& pCurrentTreeItem)
		{
			descendants.push_back(pCurrentTreeItem);
			const std::vector<std::shared_ptr<TreeItem>> children = pCurrentTreeItem->GetChildren();
			for (auto& child : children)
				RecursivelyGetChildren(child);
		};

		RecursivelyGetChildren(pTreeItem);

		return descendants;
	}

	void Tree::DrawHeaderCellContent(const TableIcon& tableIcon, const std::string& label, UI::Alignment alignment, int column) noexcept
	{
		const bool hasIcon = tableIcon.IconTextureHandle.IsValid();
		const bool hasText = !label.empty();

		const ImRect cellRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), column);
		const ImVec2 cellSize = cellRect.GetSize();
		const ImVec2 cellPos = cellRect.Min;

		ImVec2 iconSize(0.0f, 0.0f);
		if (hasIcon)
		{
			float iconHeight = cellSize.y * tableIcon.SizeWeight;
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

			const Ref<Texture> pTexture = AssetManager::Get<Texture>(tableIcon.IconTextureHandle);
			const ImVec2 imageSize(iconSize);

			ImGui::Image((ImTextureID)pTexture->GetSRV()->GetGPUHandle().ptr, imageSize, ImVec2(0, 0), ImVec2(1, 1), tableIcon.Tint);

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

	void Tree::OnBeginDraw() noexcept
	{
		m_TreeNodeToggled = false;

		if (m_pDataView->pTreeStyle->IsUsingAlternateRowColors())
		{
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBg, TreeDefaultColors::EvenRowColor);
			ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_TableRowBgAlt, TreeDefaultColors::OddRowColor);
		}

		ImGui::PushStyleColor(ImGuiCol_Header, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_HeaderHovered, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_HeaderActive, IM_COL32(0, 0, 0, 0));

		m_pDataView->pTreeInteraction->OnBeginTree();

		m_OuterRect.Min = ImGui::GetCursorScreenPos();
	}

	void Tree::Draw() noexcept
	{
		OnBeginDraw();

		if (ImGui::BeginTable(GetID(), static_cast<int>(m_Columns.size()), m_Flags))
		{
			if (m_IsFocused != ImGui::IsWindowFocused())
			{
				m_IsFocused = ImGui::IsWindowFocused();
				OnFocusChanged(m_IsFocused);
			}

			SetupHeaderAndColumns();
			HandleColumnTooltips(); 

			const std::vector<TreeDataRow> flattenedHierarchy = FlattenTree(); 

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

	void Tree::OnEndDraw() noexcept
	{
		m_OuterRect.Max = ImGui::GetCursorScreenPos();
		m_OuterRect.Max.x += ImGui::GetContentRegionAvail().x;
		m_OuterRect.Max.y -= ImGui::GetStyle().ItemSpacing.y;

		const uint32_t nrOfColorsToPop = m_pDataView->pTreeStyle->IsUsingAlternateRowColors() ? 5 : 3;
		ImGui::PopStyleColor(nrOfColorsToPop);

		m_pDataView->pTreeInteraction->OnEndTree();
	}

	void Tree::DrawRow(const std::shared_ptr<TreeItem>& pTableData, uint32_t indentationLevel) noexcept
	{
		for (uint32_t columnIndex = 0u; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);
			DrawRowCell(pTableData, columnIndex, indentationLevel);
		}
	}

	void Tree::DrawRowCell(const std::shared_ptr<TreeItem>& pTreeItem, int columnIndex, uint32_t indentationLevel)
	{
		const ImRect cellRect = ImGui::TableGetCellBgRect(ImGui::GetCurrentTable(), columnIndex);
		const ImVec2 cellSize = cellRect.GetSize();
		const ImVec2 cellPos = cellRect.Min;

		const ColumnProperties& column = m_Columns[columnIndex];
		const TreeItemStyle rowStyle = pTreeItem->GetData().ColumnStyles[columnIndex];
		const TableIcon& tableIcon = pTreeItem->GetData().ColumnIcons[columnIndex];
		const AssetHandle iconHandle = pTreeItem->GetColumnIcon(columnIndex);

		const char* label = pTreeItem->GetColumnLabel(columnIndex).c_str();

		const bool hasIcon = iconHandle.IsValid();
		const bool hasLabel = std::strcmp(label, "") != 0;

		if (column.IsTreeNode)
		{
			ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_AllowItemOverlap;

			if (pTreeItem->HasChildren())
				nodeFlags |= (ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow);
			else
				nodeFlags |= ImGuiTreeNodeFlags_Leaf;

			if (indentationLevel > 0)
				ImGui::Indent(indentationLevel * ImGui::GetTreeNodeToLabelSpacing() * 0.6f);
			
			const void* nodeID = (uint32_t*)pTreeItem.get() + columnIndex;
			ImGui::SetNextItemOpen(pTreeItem->IsExpanded());
			const bool isExpanded = ImGui::TreeNodeEx(nodeID, nodeFlags, "");
			m_TreeNodeToggled |= ImGui::IsItemToggledOpen();
			pTreeItem->SetExpanded(isExpanded);

			ImGui::SameLine();
			if (hasIcon)
			{
				const float iconHeight = cellSize.y * tableIcon.SizeWeight;
				const ImVec2 iconSize = ImVec2(iconHeight, iconHeight);
				const float iconPosY = cellPos.y + (cellSize.y - iconSize.y) * 0.5f;
				ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, iconPosY));

				const Ref<Texture> pIcon = AssetManager::Get<Texture>(iconHandle);
				ImGui::Image((ImTextureID)pIcon->GetSRV()->GetGPUHandle().ptr, iconSize, ImVec2(0, 0), ImVec2(1, 1), tableIcon.Tint);
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

				const Ref<Texture> pIcon = AssetManager::Get<Texture>(iconHandle);
				ImGui::Image((ImTextureID)pIcon->GetSRV()->GetGPUHandle().ptr, iconSize, ImVec2(0, 0), ImVec2(1, 1), tableIcon.Tint);
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

	void Tree::DetermineAndSetRowBackgroundColor(const std::shared_ptr<TreeItem>& pTreeItem) noexcept
	{
		if (pTreeItem->GetData().UseDefaultItemColor)
			return;

		const ImU32 targetColor = ImGui::GetColorU32(pTreeItem->GetData().BackgroundColor);

		for (uint32_t columnIndex = 0u; columnIndex < m_Columns.size(); ++columnIndex)
		{
			ImGui::TableSetColumnIndex(columnIndex);
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, targetColor);
		}
	}

	void Tree::SetupHeaderAndColumns() noexcept
	{
		if (m_FreezeHeader)
			ImGui::TableSetupScrollFreeze(0, 1);

		m_HeaderRect.Min = ImGui::GetCursorScreenPos();
		
		SetupColumns();
		ImGui::TableHeadersRow();

		for (int column = 0; column < m_Columns.size(); ++column)
		{
			ImGui::TableSetColumnIndex(column);
			DrawHeaderCellContent(m_Columns[column].HeaderIcon, m_Columns[column].Label, m_Columns[column].Alignment, column);
		}

		m_HeaderRect.Max = ImGui::GetCursorScreenPos();
		m_HeaderRect.Max.x += ImGui::GetContentRegionAvail().x;
	}

	void Tree::SetupColumns() noexcept
	{
		for (int columnIndex = 0; columnIndex < m_Columns.size(); ++columnIndex)
		{
			const ColumnProperties& column = m_Columns[columnIndex];
			ImGui::TableSetupColumn("", column.Flags, column.DefaultWeight);
		}
	}

	void Tree::HandleColumnTooltips() noexcept
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

	void Tree::HandleRowInteraction(const std::shared_ptr<TreeItem>& pTreeItem, uint32_t indentationLevel) noexcept
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
				m_pDataView->pTreeInteraction->OnHoveringItem(pTreeItem, columnIndex);

				if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pDataView->pTreeInteraction->OnClickedOnItem(pTreeItem, columnIndex, true);
				}
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pDataView->pTreeInteraction->OnClickedOnItem(pTreeItem, columnIndex, false);
				}
				else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
				{
					if (!m_TreeNodeToggled)
						m_pDataView->pTreeInteraction->OnReleasedMouseOnItem(pTreeItem, columnIndex);
				}
			}

			auto&& GetNonTreeNodeArrowArea = [&]() -> std::vector<ImRect>
			{
				std::vector<ImRect> toReturn;
				ImVec2 min = cellRect.Min;

				if (!(pTreeItem->HasChildren() && m_Columns[columnIndex].IsTreeNode))
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
				return toReturn;
			};

			// Set cursor to the top-left of the cell
			ImGui::SetCursorScreenPos(ImVec2(cellRect.Min.x, cellPos.y));

			const std::vector<ImRect> nonArrowAreas = GetNonTreeNodeArrowArea();
			for (uint32_t i = 0u; i < nonArrowAreas.size(); ++i)
			{
				const ImRect& area = nonArrowAreas[i];
				if (area.GetWidth() == 0)
					continue;

				// Create a unique ID for the invisible button
				char buttonID[64];
				snprintf(buttonID, sizeof(buttonID), "##cell_%p_%p", pTreeItem.get(), columnIndex + pTreeItem.get());
				
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

			if (!m_TreeNodeToggled && m_pDataView->pDragDropBehavior)
			{
				if (m_pDataView->pDragDropBehavior->BeginDragDropSource())
				{
					if (!m_pDataView->pDragDropBehavior->HasPayload())
						m_pDataView->pDragDropBehavior->SetPayload(pTreeItem);

					m_pDataView->pDragDropBehavior->EndDragDropSource();
				}

				if (m_pDataView->pDragDropBehavior->BeginDragDropTarget())
				{
					m_pDataView->pDragDropBehavior->TryPayloadDelivery(pTreeItem, "TreeItem");
					m_pDataView->pDragDropBehavior->EndDragDropTarget();
				}
			}
			
		}
	}

	float Tree::CalculateCellContentStartPosition(ImRect cellRect, UI::Alignment alignment, float contentWidth) const noexcept
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

	std::vector<Tree::TreeDataRow> Tree::FlattenTree() const noexcept
	{
		std::vector<Tree::TreeDataRow> toReturn;
		uint32_t indentationLevel = 0u;

		std::function<void(const std::vector<std::shared_ptr<TreeItem>>&, uint32_t)> AddAllChildren;
		AddAllChildren = [&](const std::vector<std::shared_ptr<TreeItem>>& children, uint32_t indentationLevel) 
			{
				for (auto& child : children)
				{
					toReturn.push_back({ child, indentationLevel });
					if (child->HasChildren() && child->IsExpanded())
						AddAllChildren(child->GetChildren(), indentationLevel + 1);
				}
			};

		for (uint32_t i = 0u; i < m_Entries.size(); ++i)
		{
			toReturn.push_back({ m_Entries[i], indentationLevel });
			if (m_Entries[i]->HasChildren() && m_Entries[i]->IsExpanded())
				AddAllChildren(m_Entries[i]->GetChildren(), indentationLevel + 1);
		}

		return toReturn;
	}

	const char* Tree::GetID() const noexcept
	{
		return m_ID;
	}

}
