#include "ITreeNode.h"
#include "UI/Widgets/IWidget.h"

namespace Relentless
{
	Ref<IBaseWidget> ITreeNode::CreateWidget() noexcept
	{
		return m_OnCreateWidgetCallback(this);
	}

	void ITreeNode::Filter(const String& filter) noexcept
	{
		m_MatchesFilter = MatchesFilterTag(filter);
	}

	const std::vector<Ref<ITreeNode>>& ITreeNode::GetChildren() const noexcept
	{
		return m_Children;
	}

	const std::vector<String>& ITreeNode::GetFilterTags() const noexcept
	{
		return m_FilterTags;
	}

	const String& ITreeNode::GetLabel() const noexcept
	{
		return m_Label;
	}

	void ITreeNode::SetFilterTags(const Span<String>& tags) noexcept
	{
		m_FilterTags = tags.Copy();
	}
}