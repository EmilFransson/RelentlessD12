#pragma once
#include "ImGui/ImguiLayer.h"
#include "UI/UI.h"
#include "TreeTypes.h"
#include "Callback/Broadcaster.h"

namespace Relentless 
{
	class TreeItem;
	class TableDataSelection;
	class TreeInteraction;
	class TreeStyle;
	class DragDropBehavior;

	struct TreeDataView
	{
		std::shared_ptr<TreeInteraction> pTreeInteraction = nullptr;
		std::shared_ptr<TreeStyle> pTreeStyle = nullptr;
		std::shared_ptr<DragDropBehavior> pDragDropBehavior = nullptr;
	};

	class Tree
	{
	public:
		
		struct ColumnProperties
		{
			std::string Label = "";
			std::string HeaderTooltip = "";
			TableIcon HeaderIcon{};
			ImGuiTableColumnFlags Flags = ImGuiTableColumnFlags_::ImGuiTableColumnFlags_None;
			float DefaultWeight = 0.0f;
			bool IsTreeNode = false;
			bool AllowSelection = true;
			UI::Alignment Alignment = UI::Alignment::Left;
		};

		struct TreeDataRow
		{
			std::shared_ptr<TreeItem> Entry = nullptr;
			uint32_t IndentationLevel = 0u;
		};

		Tree(const std::shared_ptr<TreeDataView>& dataView) noexcept;
		virtual ~Tree() noexcept;

		void Draw() noexcept;

		void SetFreezeHeader(bool state) noexcept;
		void SetFlags(ImGuiTableFlags flags);

		virtual void AddEntry(const std::shared_ptr<TreeItem>& pTreeItem) noexcept;
		virtual void RemoveEntry(const std::shared_ptr<TreeItem>& pTreeItem) noexcept;
		void AddColumn(const ColumnProperties& column) noexcept;

		[[nodiscard]] std::vector<std::shared_ptr<TreeItem>>& GetRootEntries() noexcept;
		[[nodiscard]] const std::shared_ptr<TableDataSelection>& GetTableDataSelection() const noexcept;

		void RemoveAll() noexcept;

		[[nodiscard]] bool IsFocused() const noexcept;
		[[nodiscard]] bool IsHovered() const noexcept;
		[[nodiscard]] bool IsHeaderSpaceHovered() const noexcept;
		[[nodiscard]] bool IsRowSpaceHovered() const noexcept;
		[[nodiscard]] bool IsTreeNodeToggled() const noexcept;

		[[nodiscard]] uint32_t GetNumColumns() const noexcept;
		[[nodiscard]] std::vector<std::shared_ptr<TreeItem>> GetDescendants(const std::shared_ptr<TreeItem>& pTreeItem) const noexcept;
		
		[[nodiscard]] std::vector<TreeDataRow> FlattenTree() const noexcept;

		Broadcaster<void(bool isFocused)> OnFocusChanged;

	protected:
		virtual [[nodiscard]] const char* GetID() const noexcept = 0;
		virtual void DrawHeaderCellContent(const TableIcon& tableIcon, const std::string& label, UI::Alignment alignment, int column) noexcept;
	private:
		//Drawing:
		void OnBeginDraw() noexcept;
		void OnEndDraw() noexcept;
		void DrawRow(const std::shared_ptr<TreeItem>& pTableData, uint32_t indentationLevel) noexcept;
		void DrawRowCell(const std::shared_ptr<TreeItem>& pTableData, int columnIndex, uint32_t indentationLevel);
		void DetermineAndSetRowBackgroundColor(const std::shared_ptr<TreeItem>& pTableData) noexcept;

		void SetupHeaderAndColumns() noexcept;
		void SetupColumns() noexcept;
		void HandleColumnTooltips() noexcept;
		void HandleRowInteraction(const std::shared_ptr<TreeItem>& pTableData, uint32_t indentationLevel) noexcept;

		[[nodiscard]] float CalculateCellContentStartPosition(ImRect cellRect, UI::Alignment alignment, float contentWidth) const noexcept;
	private:
		std::vector<ColumnProperties> m_Columns;
		std::vector<std::shared_ptr<TreeItem>> m_Entries;
		ImGuiTableFlags m_Flags = 0;

		bool m_IsFocused = false;
		bool m_FreezeHeader = true;
		bool m_TreeNodeToggled = false;

		std::shared_ptr<TableDataSelection> m_pSelectionContext = nullptr;
		std::shared_ptr<TreeStyle> m_pTableStyling = nullptr;
		
		std::shared_ptr<TreeDataView> m_pDataView = nullptr;

		ImRect m_OuterRect;
		ImRect m_HeaderRect;
	};
}