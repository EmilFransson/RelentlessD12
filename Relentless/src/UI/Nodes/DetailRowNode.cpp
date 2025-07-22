#include "DetailRowNode.h"

#include "UI/Details/DetailPropertyRow.h"

namespace Relentless
{
	bool DetailRowNode::MatchesFilterTag(const String& filterTag) const noexcept
	{
		return filterTag.empty() || std::any_of(m_FilterTags.begin(), m_FilterTags.end(), [&filterTag](const String& tag)
			{
				return tag.find(filterTag) != String::npos;
			});
	}
}