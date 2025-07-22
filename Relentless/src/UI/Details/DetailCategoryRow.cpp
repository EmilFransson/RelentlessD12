#include "DetailCategoryRow.h"

namespace Relentless
{
	DetailCategoryRow::DetailCategoryRow() noexcept
	{
		SetTableFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_NoPadOuterX);
		SetHeaderFlags(ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_OpenOnArrow);

		SetBorderLightColor(Colors::Normalize(20.0f, 20.0f, 20.0f, 255.0f));
		SetSeparatorColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
		SetSeparatorHoverColor(Colors::Normalize(90.0f, 90.0f, 90.0f, 255.0f));
		SetSeparatorActiveColor(Colors::Normalize(110.0f, 110.0f, 110.0f, 255.0f));

		const ImVec4& frameColor = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
		SetActiveColor(Color(frameColor.x, frameColor.y, frameColor.z, frameColor.w));
		SetHoverColor(Color(frameColor.x, frameColor.y, frameColor.z, frameColor.w));

		SetCellPadding(Vector2(20.0f, 4.0f));
		SetFont(ImGui::GetIO().Fonts->Fonts[0]);
	}

	void DetailCategoryRow::OnRender() noexcept
	{
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[2]);

		const bool isOpen = ImGui::CollapsingHeader(std::format("##{}_CollapsingHeader", (long)this).c_str(), m_HeaderFlags);

		ImGui::PopFont();

		if (!isOpen)
			return;

		const String tableID = std::format("##{}_Table", (long)this).c_str();
		m_IsRenderingTable = ImGui::BeginTable(tableID.c_str(), 3, m_TableFlags);

		if (m_IsRenderingTable)
		{
			ImGui::TableSetupColumn("NameContentColumn", ImGuiTableColumnFlags_WidthStretch, 0.8f);
			ImGui::TableSetupColumn("ValueContentColumn", ImGuiTableColumnFlags_WidthStretch, 1.0f);
			ImGui::TableSetupColumn("ResetToDefaultColumn", ImGuiTableColumnFlags_WidthStretch, 0.1f);
			//
			//ImGuiTable* pTable = ImGui::GetCurrentTable();
			//if (pTable->MinColumnWidth > 0 && pTable->Columns.Data[2].ItemWidth < 60.0f)
			//	ImGui::TableSetColumnWidth(2, 60.0f);


			//const float available = ImGui::GetColumnWidth(2);
			//
			//if (available < 100.0f)
				//
		}
	}

	void DetailCategoryRow::Finish() noexcept
	{
		if (m_IsRenderingTable)
		{
			m_IsRenderingTable = false;
			ImGui::EndTable();
		}
	}

	void DetailCategoryRow::SetHeaderFlags(int flags) noexcept
	{
		m_HeaderFlags = flags;
	}

	void DetailCategoryRow::SetCellPadding(const Vector2& padding) noexcept
	{
		m_Style.SetStyleVar(ImGuiStyleVar_CellPadding, ImVec2(padding.x, padding.y));
	}

	void DetailCategoryRow::SetBorderLightColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_TableBorderLight, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void DetailCategoryRow::SetSeparatorColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_Separator, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void DetailCategoryRow::SetSeparatorHoverColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SeparatorHovered, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void DetailCategoryRow::SetSeparatorActiveColor(const Color& color) noexcept
	{
		m_Style.SetStyleColor(ImGuiCol_SeparatorActive, ImVec4(color.R(), color.G(), color.B(), color.A()));
	}

	void DetailCategoryRow::SetTableFlags(int flags) noexcept
	{
		m_TableFlags = flags;
	}
}