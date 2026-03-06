#include "ListView.h"

#include "../Widgets/Label.h"

namespace Relentless
{
	void HeaderRow::AddColumn(const Column& newColumn) noexcept
	{
		m_Columns.push_back(newColumn);
	}

	const Column& HeaderRow::GetColumn(uint32 index) const noexcept
	{
		return m_Columns[index];
	}

	uint32 HeaderRow::GetNumColumns() const noexcept
	{
		return m_Columns.size();
	}

	void HeaderRow::OnRender() noexcept
	{
		if (m_IsPinned)
			ImGui::TableSetupScrollFreeze(0, 1);

		for (uint32 col = 0u; col < GetNumColumns(); ++col)
		{
			const Column& column = GetColumn(col);
			ImGui::TableSetupColumn("##Column", column.Flags, column.Weight);
		}

		if (!m_IsVisible)
			return;

		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		for (uint32 col = 0u; col < GetNumColumns(); ++col)
		{
			ImGui::TableSetColumnIndex(col);
			const Column& column = GetColumn(col);
			column.pBox->AssignSize({ ImGui::GetContentRegionAvail().x, 32.0f });
			column.pBox->Render();
		}
	}

	void HeaderRow::SetIsPinned(bool isPinned) noexcept
	{
		m_IsPinned = isPinned;
	}

	void HeaderRow::SetIsVisible(bool aIsVisible) noexcept
	{
		m_IsVisible = aIsVisible;
	}

}