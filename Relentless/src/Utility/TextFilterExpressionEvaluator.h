#pragma once

namespace Relentless
{
	enum class ETextFilterTextComparisonMode : uint8 { Exact = 0, Partial, StartsWith, EndsWith };

	class TextFilterExpressionEvaluator
	{
	public:
		TextFilterExpressionEvaluator() noexcept = default;
		virtual ~TextFilterExpressionEvaluator() noexcept = default;

		NO_DISCARD const String& GetFilterText() const noexcept;

		void SetFilterText(const String& filterText) noexcept;
		
		NO_DISCARD bool TestTextFilter(std::string_view text, ETextFilterTextComparisonMode comparisonMode) noexcept;
	private:
		String m_FilterText;
	};
}