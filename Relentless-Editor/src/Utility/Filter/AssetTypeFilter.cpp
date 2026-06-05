#include "AssetTypeFilter.h"

namespace Relentless
{
	bool AssetTypeFilter::IsActive() const
	{
		return std::ranges::any_of(m_Enabled, [](auto& kv) { return kv.second; });
	}

	bool AssetTypeFilter::IsEnabled(TypeIndex aType) const
	{
		return m_Enabled.contains(aType) && m_Enabled.at(aType);
	}

	bool AssetTypeFilter::PassesFilter(const AssetData& aAssetData) const
	{
		if (!IsActive()) 
			return true;
		
		const auto it = m_Enabled.find(aAssetData.Type);
		return it != m_Enabled.end() && it->second;
	}

	void AssetTypeFilter::SetEnabled(TypeIndex aType, bool aEnabled) noexcept
	{
		if (!m_Enabled.contains(aType) || m_Enabled[aType] != aEnabled)
		{
			m_Enabled[aType] = aEnabled;
			OnChanged();
		}
	}

}