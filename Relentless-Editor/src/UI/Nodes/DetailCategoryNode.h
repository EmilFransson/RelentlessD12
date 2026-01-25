#pragma once
#include "IDetailsTreeNode.h"

namespace Relentless
{
	class DetailRowNode;
	class DetailCategoryRow;

	class DetailCategoryNode : public IDetailsTreeNode
	{
	public:
		DetailCategoryNode() noexcept = default;
		virtual ~DetailCategoryNode() noexcept = default;
		
		DetailRowNode* AddChild() noexcept;
	protected:
		virtual bool MatchesFilterTag(const String& filterTag) const noexcept override;
	};
}