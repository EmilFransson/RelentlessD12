#pragma once
#include <Relentless.h>

namespace Relentless
{
	class OutlinerSceneTableData;
	class OutlinerEntityTableData;

	class OutlinerTableDataSelection : public TableDataSelection
	{
	public:
		OutlinerTableDataSelection(Table* pTable) noexcept;
		virtual ~OutlinerTableDataSelection() noexcept override = default;

		void SelectAllExpandedEntityRows() noexcept;
		[[nodiscard]] uint32_t GetNrOfSelectedEntities() const noexcept;
	private:
		virtual void OnSelected(const std::shared_ptr<TableData>& pTableData) noexcept override;
		virtual void OnDeselected(const std::shared_ptr<TableData>& pTableData) noexcept override;
		virtual void OnClicked(const std::shared_ptr<TableData>& pTableData, uint32_t column, bool doubleClicked) noexcept override;

		void OnSceneRowClicked(const std::shared_ptr<OutlinerSceneTableData>& pSceneTableData, uint32_t column) noexcept;
		void OnEntityRowClicked(const std::shared_ptr<OutlinerEntityTableData>& pEntityTableData, uint32_t column, bool doubleClicked) noexcept;
	private:
		uint32_t m_NrOfSelectedEntities = 0u;
	};
}