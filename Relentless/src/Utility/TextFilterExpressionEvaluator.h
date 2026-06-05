#pragma once
#include "Core/DLLExport.h"

namespace Relentless
{
	enum class ETextFilterTextComparisonMode : uint8 { Exact = 0, Partial, StartsWith, EndsWith };

	class RLS_API TextFilterExpressionEvaluator
	{
	public:
		TextFilterExpressionEvaluator() noexcept = default;
		virtual ~TextFilterExpressionEvaluator() noexcept = default;

		NO_DISCARD const String& GetFilterText() const noexcept;

		void SetFilterText(const String& aFilterText) noexcept;
		
		NO_DISCARD bool TestTextFilter(StringView aText, ETextFilterTextComparisonMode aComparisonMode) noexcept;
	private:
		String m_FilterText;
	};
}