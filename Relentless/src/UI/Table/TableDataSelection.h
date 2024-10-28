#pragma once

namespace Relentless
{
	class Table;
	class TableData;

	constexpr uint32_t ANY_COLUMN_HOVERED = std::numeric_limits<uint32_t>::max();

	class TableDataSelection
	{
	public:
		enum class SelectionMode : uint8_t { Single, Toggle, Range };

		TableDataSelection(Table* pTable) noexcept;
		virtual ~TableDataSelection() noexcept = default;

		[[nodiscard]] bool IsReferenceSelection(const std::shared_ptr<TableData>& pTableData) noexcept;

		virtual void OnDrawBegin() noexcept;
		virtual void OnDrawEnd() noexcept;

		void OnClickedOnRow(const std::shared_ptr<TableData>& tableData, SelectionMode selectionMode, uint32_t column, bool doubleClicked) noexcept;
		void OnReleasedOnRow(const std::shared_ptr<TableData>& tableData, SelectionMode selectionMode) noexcept;

		void Select(const std::shared_ptr<TableData>& tableData);
		void Deselect(const std::shared_ptr<TableData>& tableData) noexcept;
		void DeselectAll() noexcept;

		[[nodiscard]] bool IsSelected(const std::shared_ptr<TableData>& tableData) const noexcept;
		[[nodiscard]] uint32_t GetSelectedCount() const noexcept;
		[[nodiscard]] const std::vector<std::shared_ptr<TableData>>& GetSelected() const noexcept;

		void SetHovered(const std::shared_ptr<TableData>& tableData, uint32_t column) noexcept;
		[[nodiscard]] bool IsHovered(const std::shared_ptr<TableData>& tableData, uint32_t hoveredColumn = ANY_COLUMN_HOVERED) const noexcept;

		[[nodiscard]] bool IsAncestorToAnySelected(const std::shared_ptr<TableData>& tableData) const noexcept;
	protected:
		virtual [[nodiscard]] bool IsSelectable([[maybe_unused]] const std::shared_ptr<TableData>& tableData, [[maybe_unused]] uint32_t column, [[maybe_unused]] SelectionMode selectionMode) noexcept { return true; };
		virtual void OnSelected([[maybe_unused]] const std::shared_ptr<TableData>& tableData) noexcept {};
		virtual void OnDeselected([[maybe_unused]] const std::shared_ptr<TableData>& tableData) noexcept {};
		virtual void OnHovered([[maybe_unused]] const std::shared_ptr<TableData>& tableData) noexcept {};
		virtual void OnClicked([[maybe_unused]] const std::shared_ptr<TableData>& tableData, [[maybe_unused]] uint32_t column, [[maybe_unused]] bool doubleClicked) noexcept {};
	private:
		void SetReferenceSelection(const std::shared_ptr<TableData>& pTableData) noexcept;
	protected:
		Table* m_pTable = nullptr;
	private:
		std::vector<std::shared_ptr<TableData>> m_Selected;
		std::shared_ptr<TableData> m_pReference = nullptr;
		std::shared_ptr<TableData> m_pHovered = nullptr;
		uint32_t m_HoveredColumn = ANY_COLUMN_HOVERED;
		bool m_HoveredStateSetThisFrame = false;
	};
}

