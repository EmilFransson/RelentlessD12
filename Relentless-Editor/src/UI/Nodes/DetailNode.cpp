#include "DetailNode.h"

#include "UI/Views/TreeView.h"

namespace Relentless
{
	DetailNode::DetailNode(const char* aName) noexcept
		: m_Name{ aName }
	{
	}

	DetailNode::DetailNode(const char* aName, Ref<IPropertyHandleBase> aPropertyHandle) noexcept
		: m_Name{ aName },
		  m_pPropertyHandle{ aPropertyHandle }
	{
	}

	void DetailNode::AddChild(const Ref<DetailNode>& aChildNode) noexcept
	{
		m_Children.push_back(aChildNode);
	}

	const std::vector<Ref<DetailNode>>& DetailNode::GetChildren() const noexcept
	{
		return m_Children;
	}

	const String& DetailNode::GetName() const noexcept
	{
		return m_Name;
	}

	Ref<IPropertyHandleBase> DetailNode::GetPropertyHandle() const noexcept
	{
		return m_pPropertyHandle;
	}

	bool DetailNode::IsGroupNode() const noexcept
	{
		return m_IsGroupNode;
	}

	Ref<ITableRow> DetailNode::RequestRowWidget(const ItemInfo& aItemInfo) noexcept
	{
		return m_OnRequestRowCallback(aItemInfo);
	}

	void DetailNode::SetChildren(const std::vector<Ref<DetailNode>>& someChildren) noexcept
	{
		m_Children = someChildren;	
	}

	void DetailNode::SetIsGroupNode(bool aIsGroupNode) noexcept
	{
		m_IsGroupNode = aIsGroupNode;
	}
}