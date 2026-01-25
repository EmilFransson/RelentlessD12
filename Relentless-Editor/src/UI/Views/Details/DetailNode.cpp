#include "DetailNode.h"

#include "UI/Views/TreeView.h"

namespace Relentless
{
	DetailNode::DetailNode(const char* aName) noexcept
		: m_Name{ aName }
	{
	}

	const std::vector<Ref<DetailNode>>& DetailNode::GetChildren() const noexcept
	{
		return m_Children;
	}

	const String& DetailNode::GetName() const noexcept
	{
		return m_Name;
	}

	Ref<ITableRow> DetailNode::RequestRowWidget(const ItemInfo& aItemInfo) noexcept
	{
		return m_OnRequestRowCallback(aItemInfo);
	}

	void DetailNode::SetChildren(const std::vector<Ref<DetailNode>>& someChildren) noexcept
	{
		m_Children = someChildren;	
	}
}