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
		for (uint32 col = 0u; col < GetNumColumns(); ++col)
		{
			const Column& column = GetColumn(col);
			ImGui::TableSetupColumn(column.pLabel->GetText().c_str(), column.Flags, column.Weight);
		}

		ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
		for (uint32 col = 0u; col < GetNumColumns(); ++col)
		{
			ImGui::TableSetColumnIndex(col);
			GetColumn(col).pLabel->Render();
		}
	}

}