#pragma once

namespace Relentless
{
	class Table;
	class TableData;

	class TableInteraction
	{
	public:
		struct PayloadInfo
		{
			const char* ID = nullptr;
			const void* Data = nullptr;
			size_t Size = 0u;
			const char* TooltipLabel = nullptr;
		};

		TableInteraction(Table* pTable) noexcept;
		virtual ~TableInteraction() noexcept = default;

		virtual [[nodiscard]] bool IsDraggable([[maybe_unused]] const std::shared_ptr<TableData>& pTableData, [[maybe_unused]] uint32_t column) const noexcept { return true; }
		virtual [[nodiscard]] PayloadInfo GetPayloadInfo(const std::vector<std::shared_ptr<TableData>>& selectedRows) const noexcept = 0;

		[[nodiscard]] bool IsDragDropEnabled() const noexcept;
		void SetDragDropEnabled(bool enabled) noexcept;

		void SetFilter(std::string_view filter) noexcept;
		[[nodiscard]] bool UsesFilter() const noexcept;
	private:
		Table* m_pTable = nullptr;
		std::string m_Filter = "";
		bool m_DragDropEnabled = true;
	};
}