#pragma once
#include "ImGui/ImguiLayer.h"
#include "Assets/AssetMeta.h"
#include "UI/UI.h"

namespace Relentless 
{
	class TableData;
	class TableDataSelection;
	class TableDataSlice;

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
		
		void SetEvenRowColor(const ImVec4& evenRowColor) noexcept;
		void SetOddRowColor(const ImVec4& oddRowColor) noexcept;
		void SetRowHoverColor(const ImVec4& rowHoverColor) noexcept;

		void SetFreezeHeader(bool state) noexcept;
		void SetFlags(ImGuiTableFlags flags);
		void SetTableDataSelection(const std::shared_ptr<TableDataSelection>& tableDataSelection) noexcept;

		virtual void AddEntry(const std::shared_ptr<TableData>& pTableData) noexcept;
		void AddColumn(const ColumnProperties& column) noexcept;

		[[nodiscard]] std::vector<std::shared_ptr<TableData>>& GetRootEntries() noexcept;

		void RemoveAll() noexcept;

		[[nodiscard]] bool IsFocused() const noexcept;
		[[nodiscard]] bool IsHovered() const noexcept;
		[[nodiscard]] bool IsHeaderSpaceHovered() const noexcept;
		[[nodiscard]] bool IsRowSpaceHovered() const noexcept;
		
		[[nodiscard]] std::vector<TableDataRow> FlattenTree() const noexcept;
	protected:
		virtual [[nodiscard]] const char* GetID() const noexcept = 0;
		virtual void DrawHeaderCellContent(const TableIcon& tableIcon, const std::string& label, UI::Alignment alignment, int column) noexcept;
	private:
		//Drawing:
		void OnBeginDraw() noexcept;
		void OnEndDraw() noexcept;
		void DrawRow(const std::shared_ptr<TableData>& tableData, uint32_t indentationLevel) noexcept;
		void DrawRowCell(const std::shared_ptr<TableData>& tableData, int columnIndex, uint32_t indentationLevel);
		void DetermineAndSetRowBackgroundColor(const std::shared_ptr<TableData>& tableData) noexcept;

		void SetupHeaderAndColumns() noexcept;
		void SetupColumns() noexcept;
		void HandleColumnTooltips() noexcept;
		void HandleRowInteraction(const std::shared_ptr<TableData>& tableData) noexcept;

	private:
		std::vector<ColumnProperties> m_Columns;
		ImGuiTableFlags m_Flags = 0;

		bool m_IsFocused = false;
		bool m_FreezeHeader = true;

		ImVec4 m_EvenRowColor = ImVec4(21.0f / 255.0f, 21.0f / 255.0f, 21.0f / 255.0f, 255.0f / 255.0f);
		ImVec4 m_OddRowColor = ImVec4(26.0f / 255.0f, 26.0f / 255.0f, 26.0f / 255.0f, 255.0f / 255.0f);
		ImVec4 m_RowHoverColor = ImVec4(36.0f / 255.0f, 36.0f / 255.0f, 36.0f / 255.0f, 1.0f);
		ImVec4 m_RowSelectedColor = ImVec4(64.0f / 255.0f, 87.0f / 255.0f, 111.0f / 255.0f, 1.0f);
		ImVec4 m_RowSelectedFocusedColor = ImVec4(30.0f / 255.0f, 120.0f / 255.0f, 255.0f / 255.0f, 200.0f / 255.0f);
		ImVec4 m_RowAncestorToSelectedColor = ImVec4(44.0f / 255.0f, 50.0f / 255.0f, 58.0f / 255.0f, 1.0f);

		std::shared_ptr<TableDataSelection> m_SelectionContext = nullptr;
		std::vector<std::shared_ptr<TableData>> m_Entries;

		ImRect m_OuterRect;
		ImRect m_HeaderRect;

		bool m_TreeNodeToggled = false;
	};
}