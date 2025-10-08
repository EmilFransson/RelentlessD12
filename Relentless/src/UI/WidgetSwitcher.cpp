#include "WidgetSwitcher.h"

namespace Relentless
{
	float WidgetSwitcher::CalcDesiredWidth() const noexcept
	{
		return m_ActiveIndex == -1 ? 0.0f : m_Widgets[m_ActiveIndex]->CalcDesiredWidth();
	}

	Ref<IBaseWidget> WidgetSwitcher::GetActiveWidget() const noexcept
	{
		if (m_ActiveIndex != -1)
			return m_Widgets[m_ActiveIndex];

		return nullptr;
	}

	int32 WidgetSwitcher::GetActiveWidgetIndex() noexcept
	{
		return m_ActiveIndex;
	}

	uint32 WidgetSwitcher::GetNumWidgets() const noexcept
	{
		return static_cast<uint32>(m_Widgets.size());
	}

	Ref<IBaseWidget> WidgetSwitcher::GetWidget(uint32 aSlotIndex) const noexcept
	{
		if (aSlotIndex < GetNumWidgets())
			return m_Widgets[aSlotIndex];

		return nullptr;
	}

	int32 WidgetSwitcher::GetWidgetIndex(Ref<IBaseWidget> aWidget) const noexcept
	{
		for (uint32 i = 0u; i < m_Widgets.size(); ++i)
		{
			if (m_Widgets[i] == aWidget)
				return static_cast<int32>(i);
		}

		return -1;
	}

	void WidgetSwitcher::OnRender() noexcept
	{
		if (m_ActiveIndex == -1)
			return;

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		m_Widgets[m_ActiveIndex]->Render();

		ImGui::PopStyleVar();
	}

	void WidgetSwitcher::SetActiveWidget(Ref<IBaseWidget> aWidget) noexcept
	{
		for (uint32 i = 0u; i < m_Widgets.size(); ++i)
		{
			if (m_Widgets[i] == aWidget)
			{
				m_ActiveIndex = static_cast<int32>(i);
				break;
			}
		}
	}

	void WidgetSwitcher::SetActiveWidgetIndex(uint32 aSlotIndex) noexcept
	{
		if (aSlotIndex < GetNumWidgets())
			m_ActiveIndex = aSlotIndex;
	}
}