#pragma once
namespace Relentless
{
	class Table;
	class TableData;

	class TableDataSlice
	{
	public:
		explicit TableDataSlice(Table* table, TableData* pOwner) noexcept;
		virtual ~TableDataSlice() noexcept = default;
		void Add(const std::shared_ptr<TableData>& pTableData) noexcept;
		virtual void OnEntryAdded([[maybe_unused]] const std::shared_ptr<TableData>& pTableData) noexcept {};
		const std::vector<std::shared_ptr<TableData>>& GetData() const;
		const TableData* GetOwner() const noexcept;
		Table* GetTable() const noexcept;
	private:
		Table* m_pTable = nullptr;
		TableData* m_pOwningData = nullptr;
		std::vector<std::shared_ptr<TableData>> m_SliceDatas;
	};
}
