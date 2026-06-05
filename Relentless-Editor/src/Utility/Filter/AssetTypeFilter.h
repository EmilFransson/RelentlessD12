#pragma once
#include "IAssetFilter.h"

namespace Relentless
{
	class AssetTypeFilter : public IAssetFilter
	{
	public:
		NO_DISCARD virtual bool IsActive() const override;
		NO_DISCARD bool IsEnabled(TypeIndex aType) const;

		NO_DISCARD virtual bool PassesFilter(const AssetData& aAssetData) const override;

		void SetEnabled(TypeIndex aType, bool aEnabled) noexcept;
	private:
		std::unordered_map<TypeIndex, bool> m_Enabled;
	};
}