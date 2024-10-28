#pragma once
#include <Relentless.h>

namespace Relentless
{
	class OutlinerEntityTableData;
	class OutlinerFolderTableData;
	class OutlinerSceneTableData;

	class OutlinerTableDataSelection : public TableDataSelection
	{
	public:
		OutlinerTableDataSelection(Table* pTable) noexcept;
		virtual ~OutlinerTableDataSelection() noexcept override = default;

		void SelectAllExpandedEntityRows() noexcept;
		[[nodiscard]] uint32_t GetNrOfSelectedEntities() const noexcept;
		[[nodiscard]] const std::vector<entity>& GetSelectedEntities() const noexcept;
		void DeselectAllEntities() noexcept;
	private:
		virtual void OnSelected(const std::shared_ptr<TableData>& pTableData) noexcept override;
		virtual void OnDeselected(const std::shared_ptr<TableData>& pTableData) noexcept override;
		virtual void OnClicked(const std::shared_ptr<TableData>& pTableData, uint32_t column, bool doubleClicked) noexcept override;
		bool IsSelectable(const std::shared_ptr<TableData>& tableData, uint32_t column, SelectionMode selectionMode) noexcept override;
	private:
		std::vector<entity> m_SelectedEntities;
	};
}