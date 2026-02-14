#include "Border.h"

namespace Relentless
{
	Ref<IBaseWidget> Border::GetContent() const noexcept
	{
		return m_pContent;
	}

	void Border::OnRender() noexcept
	{
		if (!m_pContent) 
			return;

		ImDrawList* pDrawList = ImGui::GetWindowDrawList();
		if (!pDrawList)
			return;

		pDrawList->ChannelsSplit(2);
		pDrawList->ChannelsSetCurrent(1);

		ImGui::BeginGroup();
		m_pContent->Render();
		ImGui::EndGroup();

		const ImVec2 min = ImGui::GetItemRectMin();
		const ImVec2 max = ImGui::GetItemRectMax();

		pDrawList->ChannelsSetCurrent(0);
		pDrawList->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(0.1f, 0.1f, 0.1f, 1.0f)));

		pDrawList->ChannelsMerge();
	}

	Vector2 Border::ReportSize() const noexcept
	{
		return m_pContent ? m_pContent->ReportSize() : Vector2::Zero;
	}
}