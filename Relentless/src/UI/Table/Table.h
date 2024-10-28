#pragma once
#include "ImGui/ImguiLayer.h"
#include "Assets/AssetMeta.h"
#include "UI/UI.h"

namespace Relentless 
{
	class TableData;
	class TableDataSelection;
	class TableDataSlice;
	class TableInteraction;
	class TableStyling;

	class Table
	{
	public:
		struct TableIcon
		{
			AssetHandle IconTextureHandle = NULL_HANDLE;
			ImVec2 SizeWeight{0.5f, 0.5f};
			ImVec4 Tint{1.0f, 1.0f, 1.0f, 1.0f};
		};

		struct ColumnProperties
		{
			std::string Name = "";
			std::string HeaderTooltip = "";
			TableIcon HeaderIcon{};
			ImGuiTableColumnFlags Flags = ImGuiTableColumnFlags_::ImGuiTableColumnFlags_None;
			float DefaultWeight = 0.0f;
			bool IsTreeNode = false;
			bool AllowSelection = true;
			UI::Alignment Alignment = UI::Alignment::Left;
		};

		struct TableDataRow
		{
			std::shared_ptr<TableData> Entry = nullptr;
			uint32_t IndentationLevel = 0u;
		};

		Table() noexcept;
		virtual ~Table() noexcept;

		void Draw() noexcept;

		void SetFreezeHeader(bool state) noexcept;
		void SetFlags(ImGuiTableFlags flags);
		void SetTableDataSelection(const std::shared_ptr<TableDataSelection>& tableDataSelection) noexcept;
		void SetTableStyling(const std::shared_ptr<TableStyling>& pTableStyling) noexcept;
		void SetTableInteraction(const std::shared_ptr<TableInteraction>& pTableInteraction) noexcept;

		virtual void AddEntry(const std::shared_ptr<TableData>& pTableData) noexcept;
		void AddColumn(const ColumnProperties& column) noexcept;

		[[nodiscard]] std::vector<std::shared_ptr<TableData>>& GetRootEntries() noexcept;
		[[nodiscard]] const std::shared_ptr<TableDataSelection>& GetTableDataSelection() const noexcept;

		void RemoveAll() noexcept;
		void ClearAllSelections() noexcept;

		[[nodiscard]] bool IsFocused() const noexcept;
		[[nodiscard]] bool IsHovered() const noexcept;
		[[nodiscard]] bool IsHeaderSpaceHovered() const noexcept;
		[[nodiscard]] bool IsRowSpaceHovered() const noexcept;
		[[nodiscard]] bool IsTreeNodeToggled() const noexcept;
		
		[[nodiscard]] std::vector<TableDataRow> FlattenTree() const noexcept;
	protected:
		virtual [[nodiscard]] const char* GetID() const noexcept = 0;
		virtual void DrawHeaderCellContent(const TableIcon& tableIcon, const std::string& label, UI::Alignment alignment, int column) noexcept;
	private:
		//Drawing:
		void OnBeginDraw() noexcept;
		void OnEndDraw() noexcept;
		void DrawRow(const std::shared_ptr<TableData>& pTableData, uint32_t indentationLevel) noexcept;
		void DrawRowCell(const std::shared_ptr<TableData>& pTableData, int columnIndex, uint32_t indentationLevel);
		void DetermineAndSetRowBackgroundColor(const std::shared_ptr<TableData>& pTableData) noexcept;

		void SetupHeaderAndColumns() noexcept;
		void SetupColumns() noexcept;
		void HandleColumnTooltips() noexcept;
		void HandleRowInteraction(const std::shared_ptr<TableData>& pTableData, uint32_t indentationLevel) noexcept;

		[[nodiscard]] float CalculateCellContentStartPosition(ImRect cellRect, UI::Alignment alignment, float contentWidth) const noexcept;
	private:
		std::vector<ColumnProperties> m_Columns;
		std::vector<std::shared_ptr<TableData>> m_Entries;
		ImGuiTableFlags m_Flags = 0;

		bool m_IsFocused = false;
		bool m_FreezeHeader = true;
		bool m_TreeNodeToggled = false;

		std::shared_ptr<TableDataSelection> m_pSelectionContext = nullptr;
		std::shared_ptr<TableStyling> m_pTableStyling = nullptr;
		std::shared_ptr<TableInteraction> m_pTableInteraction = nullptr;

		ImRect m_OuterRect;
		ImRect m_HeaderRect;
	};
}