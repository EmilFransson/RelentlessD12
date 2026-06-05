#pragma once
#include "IAssetFilter.h"

namespace Relentless
{
	class AssetSourceFilter : public IAssetFilter
	{
	public:
		NO_DISCARD virtual bool IsActive() const override;
		NO_DISCARD bool IsVisible(EAssetSourceType aAssetSourceType) const;

		NO_DISCARD virtual bool PassesFilter(const AssetData& aAssetData) const override;

		void SetSourceVisible(EAssetSourceType aAssetSourceType, bool aState);
	private:
		std::unordered_map<EAssetSourceType, bool> m_Visible; 
	};
}