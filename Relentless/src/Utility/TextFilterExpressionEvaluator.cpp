#include "TextFilterExpressionEvaluator.h"

#include "StringUtils.h"

namespace Relentless
{
	const String& TextFilterExpressionEvaluator::GetFilterText() const noexcept
	{
		return m_FilterText;
	}

	void TextFilterExpressionEvaluator::SetFilterText(const String& filterText) noexcept
	{
		m_FilterText = StringUtils::ToLower(filterText);
	}

	bool TextFilterExpressionEvaluator::TestTextFilter(std::string_view text, ETextFilterTextComparisonMode comparisonMode) noexcept
	{
		if (m_FilterText.empty())
			return true;

		const String comparator = StringUtils::ToLower(String(text));

		switch (comparisonMode)
		{
		case ETextFilterTextComparisonMode::Exact:
			return comparator == m_FilterText;
		case ETextFilterTextComparisonMode::Partial:
			return comparator.find(m_FilterText) != String::npos;
		case ETextFilterTextComparisonMode::StartsWith:
			return comparator.starts_with(m_FilterText);
		case ETextFilterTextComparisonMode::EndsWith:
			return comparator.ends_with(m_FilterText);
		}
	}
}
