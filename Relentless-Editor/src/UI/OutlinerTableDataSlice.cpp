#include "OutlinerTableDataSlice.h"
#include "OutlinerTableData.h"
#include "OutlinerTable.h"

namespace Relentless
{
	OutlinerTableDataSlice::OutlinerTableDataSlice(Table* table, TableData* pOwner) noexcept
		: TableDataSlice(table, pOwner)
	{

	}

	void OutlinerTableDataSlice::OnEntryAdded(const std::shared_ptr<TableData>& pTableData) noexcept
	{
		//if (const OutlinerEntityTableData* pEntityTableData = dynamic_cast<const OutlinerEntityTableData*>(GetOwner()))
		//{
		//	if (std::shared_ptr<OutlinerEntityTableData> pOtherEntityTableData = dynamic_pointer_cast<OutlinerEntityTableData>(pTableData))
		//	{
		//		OutlinerTable* pTable = static_cast<OutlinerTable*>(GetTable());
		//		const entity thisID = pEntityTableData->GetEntityID();
		//		const entity otherID = pOtherEntityTableData->GetEntityID();
		//		Scene* pScene = pTable->GetScene();
		//		pScene->AttachEntity(otherID, thisID);
		//	}
		//}
	}

}