#include "EntityDetailRow.h"

namespace Relentless
{
	const Color& EntityDetailRow::GetBackgroundColor() const noexcept
	{
		return Colors::Transparent;
	}

	uint32 EntityDetailRow::GetNumColumns() noexcept
	{
		return 2u;
	}

	bool EntityDetailRow::IsDragDropEligible() noexcept
	{
		return false;
	}

	void EntityDetailRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		if (aColumn == 0)
			ImGui::Indent();

		m_ColumnWidgets[aColumn]->Render();

		if (aColumn == 0)
			ImGui::Unindent();
	}

	Vector2 EntityDetailRow::ReportSize() const noexcept
	{
		const Vector2 firstSize = m_ColumnWidgets[0]->ReportSize();
		const Vector2 secondSize = m_ColumnWidgets[1]->ReportSize();
		return Vector2(firstSize.x + secondSize.x, Math::Max(firstSize.y, secondSize.y));
	}

	float EntityDetailRow::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}
}
