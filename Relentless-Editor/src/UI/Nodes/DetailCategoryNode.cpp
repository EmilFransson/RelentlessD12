#include "DetailCategoryNode.h"
#include "DetailRowNode.h"
#include "UI/Details/DetailCategoryRow.h"

namespace Relentless
{
	DetailRowNode* DetailCategoryNode::AddChild() noexcept
	{
		m_Children.push_back(new DetailRowNode());
		return static_cast<DetailRowNode*>(m_Children.back().Get());
	}

	bool DetailCategoryNode::MatchesFilterTag(const String& filterTag) const noexcept
	{
		if (filterTag.empty())
			return true;

		bool matchesFilter = false;

		for (uint32 i = 0u; i < m_Children.size() && matchesFilter == false; ++i)
			matchesFilter |= m_Children[i]->MatchesFilterTag(filterTag);

		if (!matchesFilter)
		{
			matchesFilter = std::any_of(m_FilterTags.begin(), m_FilterTags.end(), [&filterTag](const String& tag)
				{
					return tag.find(filterTag) != String::npos;
				});
		}

		return matchesFilter;
	}
}
