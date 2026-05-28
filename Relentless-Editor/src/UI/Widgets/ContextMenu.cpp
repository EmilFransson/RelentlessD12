#include "ContextMenu.h"


#include "UI/Widgets/IBaseWidget.h"
#include "UI/Widgets/SubMenuRow.h"

namespace Relentless
{
	ContextMenu::ContextMenu(bool aIsSubMenu) noexcept
		: m_IsSubMenu(aIsSubMenu)
	{
		SetHoverFlags(ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenOverlapped);

		m_pRoot = RLS_NEW VerticalBox();
		m_pRoot->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);
	}

	ContextMenu::~ContextMenu() noexcept
	{
		if (m_IsOpen)
			OnClosed();
	}

	void ContextMenu::AddRows(const std::vector<Ref<IBaseWidget>>& someRows) noexcept
	{
		for (const Ref<IBaseWidget>& pRow : someRows)
			m_pRoot->AddWidget(pRow);
	}

	void ContextMenu::OnRender() noexcept
	{
		if (!m_pRoot) 
			return;

		if (m_IsSubMenu)
		{
			RenderRows();
			return;
		}

		if (!m_IsOpen)
		{
			ImGui::OpenPopup("##ContextMenu");
			m_IsOpen = true;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

		const Vector2 size = m_pRoot->ReportSize();
		const ImVec2 winPadding = ImGui::GetStyle().WindowPadding;
		const float totalWidth = size.x + winPadding.x * 2.0f;
		const float totalHeight = size.y + winPadding.y * 2.0f;
		ImGui::SetNextWindowSize(ImVec2(totalWidth, totalHeight), ImGuiCond_Always);

		const bool isOpen = ImGui::BeginPopup("##ContextMenu", ImGuiWindowFlags_NoMove);
		ImGui::PopStyleVar();

		if (!isOpen)
		{
			if (m_IsOpen)
			{
				m_IsOpen = false;
				OnClosed();
			}
			return;
		}

		RenderRows();
		ImGui::EndPopup();
	}

	Vector2 ContextMenu::ReportSize() const noexcept
	{
		if (m_pRoot)
			return m_pRoot->ReportSize();

		return Vector2::Zero;
	}

	void ContextMenu::RenderRows() noexcept
	{
		const Vector2 size = m_pRoot->ReportSize();
		m_pRoot->AssignSize(Vector2(size.x, size.y));
		m_pRoot->Render();
	}
}
