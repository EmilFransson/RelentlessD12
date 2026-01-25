#pragma once
#include "IDetailsTreeNode.h"
#include "UI/Widgets/IWidget.h"

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