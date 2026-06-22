#include "AssetTextFilter.h"

namespace Relentless
{
	AssetTextFilter::AssetTextFilter() noexcept
	{
		m_pEvaluator = MakeUnique<TextFilterExpressionEvaluator>();
	}

	const String& AssetTextFilter::GetFilterText() const noexcept
	{
		return m_pEvaluator->GetFilterText();
	}

	bool AssetTextFilter::IsActive() const
	{
		return !m_pEvaluator->GetFilterText().empty();
	}

	bool AssetTextFilter::PassesFilter(const AssetData& aAssetData) const
	{
		if (!IsActive()) 
			return true;

		return m_pEvaluator->TestTextFilter(aAssetData.Name, ETextFilterTextComparisonMode::Partial) 
			|| m_pEvaluator->TestTextFilter(m_AssetDefinitionRegistry.GetDefinitionForAsset(aAssetData)->GetAssetDisplayName(), ETextFilterTextComparisonMode::Partial);
	}

	void AssetTextFilter::SetTextFilter(const String& aText) noexcept
	{
		m_pEvaluator->SetFilterText(aText);
		OnChanged();
	}
}