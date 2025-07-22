#pragma once
#include "../IWidget.h"
#include "IDetailsTreeNode.h"

namespace Relentless
{
	class DetailPropertyRow;

	class DetailRowNode : public IDetailsTreeNode
	{
	public:
		friend class DetailCategoryNode;

		DetailRowNode() noexcept = default;
	protected:
		virtual bool MatchesFilterTag(const String& filterTag) const noexcept override;
	};
}