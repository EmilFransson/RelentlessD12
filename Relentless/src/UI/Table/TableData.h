#pragma once
#include "Assets/AssetMeta.h"

namespace Relentless
{
	class Table;
	class TableDataSlice;

	class TableData
	{
	public:
		TableData(Table* pTable) noexcept;
		virtual ~TableData() noexcept = default;
		virtual [[nodiscard]] const char* GetColumnString([[maybe_unused]] uint32_t columnIndex) const noexcept { return ""; };
		virtual [[nodiscard]] const char* GetColumnTooltip([[maybe_unused]] uint32_t columnIndex) const noexcept { return ""; }
		virtual [[nodiscard]] AssetHandle GetColumnIcon([[maybe_unused]] uint32_t columnIndex) const noexcept { return NULL_HANDLE; }

		void AddChild(const std::shared_ptr<TableData>& pTableData) noexcept;
		[[nodiscard]] std::unique_ptr<TableDataSlice>& GetSlice() noexcept { return m_pSlice; }
		[[nodiscard]] const std::unique_ptr<TableDataSlice>& GetConstSlice() const noexcept { return m_pSlice; }

		[[nodiscard]] bool HasChildren() const noexcept;
		void SetExpanded(bool expandState) noexcept { m_IsExpanded = expandState; }
		[[nodiscard]] bool IsExpanded() const noexcept { return m_IsExpanded; }
		
		void SetTableDataSlice(std::unique_ptr<TableDataSlice>&& pSlice) noexcept;

	private:
		Table* m_pTable = nullptr;
		std::unique_ptr<TableDataSlice> m_pSlice = nullptr;
		bool m_IsExpanded = true;
	};
}
