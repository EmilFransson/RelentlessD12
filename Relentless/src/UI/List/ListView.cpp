#include "ListView.h"

#include "UI/Label.h"

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

		static std::array<ImGuiID, 3> ColumnIDs;
		ColumnIDs[0] = ImGui::GetID("MyTable|ColA");
		ColumnIDs[1] = ImGui::GetID("MyTable|ColB");
		ColumnIDs[2] = ImGui::GetID("MyTable|ColC");

		for (uint32 col = 0u; col < GetNumColumns(); ++col)
		{
			const Column& column = GetColumn(col);
			ImGui::TableSetupColumn(column.pLabel->GetText().c_str(), column.Flags, column.Weight, ColumnIDs[col]);
		}

		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		for (uint32 col = 0u; col < GetNumColumns(); ++col)
		{
			ImGui::TableSetColumnIndex(col);
			GetColumn(col).pLabel->Render();
		}
	}

	void HeaderRow::SetIsPinned(bool isPinned) noexcept
	{
		m_IsPinned = isPinned;
	}

}