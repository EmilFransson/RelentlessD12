#pragma once
#include <Relentless.h>

namespace Relentless
{
	class Scene;

	class OutlinerTableStyling : public TableStyling
	{
	public:
		OutlinerTableStyling(TableDataSelection* pTableDataSelection, Scene* pScene) noexcept;
		virtual ~OutlinerTableStyling() noexcept override = default;

		const TableRowStyle GetRowStyle(const std::shared_ptr<TableData>& pTableData, uint32_t column) const noexcept override;
	private:
		Scene* m_pScene = nullptr;
	};
}