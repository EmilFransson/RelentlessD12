#include "AssetSourceFilter.h"

namespace Relentless
{
	bool AssetSourceFilter::IsActive() const
	{
		return std::ranges::any_of(m_Visible, [](auto& keyValue) { return !keyValue.second; });
	}

	bool AssetSourceFilter::IsVisible(EAssetSourceType aAssetSourceType) const
	{
		const auto it = m_Visible.find(aAssetSourceType);
		return it == m_Visible.end() || it->second;
	}

	bool AssetSourceFilter::PassesFilter(const AssetData& aAssetData) const
	{
		if (!IsActive()) 
			return true;
		
		const auto it = m_Visible.find(aAssetData.Source);
		return it == m_Visible.end() || it->second;
	}

	void AssetSourceFilter::SetSourceVisible(EAssetSourceType aAssetSourceType, bool aState)
	{
		const bool wasVisible = IsVisible(aAssetSourceType);

		if (wasVisible == aState) 
			return;

		m_Visible[aAssetSourceType] = aState;
		OnChanged();
	}

}