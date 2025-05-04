#include "VerticalBox.h"

namespace Relentless
{
	VerticalBox::VerticalBox(std::string_view id) noexcept
		: IWidget{id}
	{
	}

	void VerticalBox::Add(Ref<IWidget> pWidget) noexcept
	{
		RLS_ASSERT(!HasWidget(pWidget), "[VerticalBox::Add] Widget already assigned as child.");
		m_Children.push_back(pWidget);
	}

	bool VerticalBox::HasWidget(Ref<IWidget> pWidget) noexcept
	{
		return std::find(m_Children.begin(), m_Children.end(), pWidget) != m_Children.end();
	}

	void VerticalBox::OnRender() noexcept
	{
		for (auto& pChild : m_Children)
			pChild->Render();
	}
}
