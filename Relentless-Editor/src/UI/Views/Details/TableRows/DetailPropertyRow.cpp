#include "DetailPropertyRow.h"

namespace Relentless
{
	DetailPropertyRow::DetailPropertyRow() noexcept
	{
		m_ColumnWidgets2.resize(GetNumColumns());
	}

	DetailPropertyRow::~DetailPropertyRow() noexcept
	{
		OnDestroy();
	}

	const Color& DetailPropertyRow::GetBackgroundColor() const noexcept
	{
		return Colors::Transparent;
	}

	uint32 DetailPropertyRow::GetNumColumns() noexcept
	{
		return 3u;
	}

	bool DetailPropertyRow::IsDragDropEligible() noexcept
	{
		return false;
	}

	void DetailPropertyRow::OnRenderColumn(uint32 aColumn) noexcept
	{
		if (aColumn == 0u)
		{
			for (uint32 i = 0u; i < m_IndentationLevel; ++i)
				ImGui::Indent();
		}

		m_ColumnWidgets2[aColumn]->AssignSize({ ImGui::GetContentRegionAvail().x, ReportSize().y });
		m_ColumnWidgets2[aColumn]->Render();

		if (aColumn == 0u)
		{
			for (uint32 i = 0u; i < m_IndentationLevel; ++i)
				ImGui::Unindent();
		}
	}

	Vector2 DetailPropertyRow::ReportSize() const noexcept
	{
		Vector2 size = Vector2::Zero;

		for (const auto& pWidget : m_ColumnWidgets2)
		{
			size.x += pWidget ? pWidget->ReportSize().x : 0.0f;
			size.y = Math::Max(size.y, pWidget ? pWidget->ReportSize().y : 0.0f);
		}

		return size;
	}

	void DetailPropertyRow::SetNameContent(Ref<IBaseWidget> aWidget) noexcept
	{
		m_ColumnWidgets2[0] = aWidget;
	}

	void DetailPropertyRow::SetValueContent(Ref<IBaseWidget> aWidget) noexcept
	{
		m_ColumnWidgets2[1] = aWidget;
	}

	void DetailPropertyRow::SetResetContent(Ref<IBaseWidget> aWidget) noexcept
	{
		m_ColumnWidgets2[2] = aWidget;

	}
}
