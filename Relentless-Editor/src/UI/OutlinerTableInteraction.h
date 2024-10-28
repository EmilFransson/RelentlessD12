#pragma once
#include <Relentless.h>

namespace Relentless
{
	class OutlinerTableInteraction : public TableInteraction
	{
	public:
		OutlinerTableInteraction(Table* pTable) noexcept;
		virtual ~OutlinerTableInteraction() noexcept override = default;
	private:
		bool IsDraggable(const std::shared_ptr<TableData>& pTableData, uint32_t column) const noexcept override;
		PayloadInfo GetPayloadInfo(const std::vector<std::shared_ptr<TableData>>& selectedRows) const noexcept override;
	private:
		int m_Payload = 13;
	};
}