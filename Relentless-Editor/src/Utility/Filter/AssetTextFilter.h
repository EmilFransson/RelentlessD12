#pragma once
#include "IAssetFilter.h"

namespace Relentless
{
	class AssetTextFilter : public IAssetFilter
	{
	public:
		AssetTextFilter() noexcept;

		NO_DISCARD virtual bool IsActive() const override;
		
		NO_DISCARD virtual bool PassesFilter(const AssetData& aAssetData) const override;

		void SetTextFilter(const String& aText) noexcept;
	private:
		UniquePtr<TextFilterExpressionEvaluator> m_pEvaluator;
	};
}