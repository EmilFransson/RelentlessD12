#include "AssetFilterCollection.h"

namespace Relentless
{
	bool AssetFilterCollection::PassesAll(const AssetData& aAssetData) const
	{
		for (const auto&[type, pFilter] : m_Filters)
		{
			if (pFilter->IsActive() && !pFilter->PassesFilter(aAssetData))
				return false;
		}

		return true;
	}

}