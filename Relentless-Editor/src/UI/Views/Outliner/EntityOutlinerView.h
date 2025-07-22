#pragma once
#include <Relentless.h>
#include "../../../Core/EntityFilters.h"

namespace Relentless
{
	struct OutlinerListItem : public RefCounted<OutlinerListItem>
	{
		entity Entity = NULL_ENTITY;
		std::shared_ptr<EntityFilter> pFilter = nullptr;

		NO_DISCARD static const String& GetEntityTypeAsString() { static String str = "Entity";  return str; }
		NO_DISCARD static const String& GetFilterTypeAsString() { static String str = "Filter"; return str; }

		NO_DISCARD bool IsEntityItem() const noexcept { return Entity != NULL_ENTITY; }
		NO_DISCARD bool IsFilterItem() const noexcept { return pFilter != nullptr; }
	};

	class OutlinerTableRow : public ITableRow
	{
	public:
		OutlinerTableRow(ListView<Ref<OutlinerListItem>>* pListView) noexcept
			: m_pOwningListView{pListView}
		{
		}

		NO_DISCARD float CalcDesiredWidth() const noexcept override
		{
			return 0.0f;
		}

		NO_DISCARD Ref<IBaseWidget> GetWidget(uint8 column)
		{
			return m_ColumnWidgets[column];
		}

		void OnRender() noexcept override
		{
			if (!m_pOwningListView)
				return;

			const Ref<OutlinerListItem>& item = m_pOwningListView->GetItemFromWidget(this);

			if (!m_Selected && m_pOwningListView->IsItemSelected(item))
			{
				m_Selected = true;
				SetBackgroundColor(Colors::OffRed);
			}
			else if (m_Selected && !m_pOwningListView->IsItemSelected(item))
			{
				m_Selected = false;
				SetBackgroundColor(Colors::Transparent);
			}

			ImGui::TableNextRow();

			for (uint32 column = 0u; column < 3; ++column)
			{
				ImGui::TableSetColumnIndex(column);

				if (column == 0) // only once per row!
				{
					ImGuiTable* table = ImGui::GetCurrentTable();
					if (!table)
						return;

					const int colCount = table->ColumnsCount;
					const float rowHeight = ImGui::GetFrameHeightWithSpacing();

					const float minX = table->Columns[0].MinX;
					const float maxX = table->Columns[colCount - 1].MaxX;
					ImVec2 rowStart = ImVec2(minX, ImGui::GetCursorScreenPos().y);
					ImVec2 rowEnd = ImVec2(maxX, rowStart.y + rowHeight);

					// --- Switch to background channel so we can draw outside column
					ImGui::TablePushBackgroundChannel();

					const bool isHovering = ImGui::IsMouseHoveringRect(rowStart, rowEnd);
					ImGui::GetWindowDrawList()->AddRectFilled(rowStart, rowEnd, ImGui::ColorConvertFloat4ToU32(ImVec4(this->m_BackgroundColor.x, this->m_BackgroundColor.y, this->m_BackgroundColor.z, this->m_BackgroundColor.w)));

					// Optional: row highlight
					if (!m_Hovered && isHovering)
					{
						m_Hovered = true;
						m_OnMouseEnterCallback.ExecuteIfSet();
					}
					else if (m_Hovered && !isHovering)
					{
						m_Hovered = false;
						m_OnMouseExitCallback.ExecuteIfSet();
					}

					ImGui::TablePopBackgroundChannel();

					// Restore cursor for first column drawing
					ImGui::SetCursorScreenPos(rowStart);
				}

				ImVec2 currentPos = ImGui::GetCursorPos();
				ImGui::SetCursorPos({ currentPos.x + m_Margins[column].Left, currentPos.y + m_Margins[column].Top });

				m_ColumnWidgets[column]->Render();
			}

			if (m_Hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnDoubleClickedCallback.ExecuteIfSet();
			else if (m_Hovered && !m_ColumnWidgets[0]->IsHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
				m_OnClickedCallback.ExecuteIfSet();
		}

		void SetMargin(const FloatRect& margin, uint32 column) noexcept
		{
			m_Margins[column] = margin;
		}

		template<typename WidgetType>
		WidgetType* SetWidget(WidgetType* pWidget, uint8 column) noexcept
		{
			static_assert(std::is_base_of<IBaseWidget, WidgetType>::value, "[OutlinerTableRow::SetWidget]: WidgetType is not derived from IWidget.");

			m_ColumnWidgets[column] = Ref<WidgetType>(pWidget);
			return pWidget;
		}

	private:
		std::array<Ref<IBaseWidget>, 3> m_ColumnWidgets;
		std::array<FloatRect, 3> m_Margins;

		ListView<Ref<OutlinerListItem>>* m_pOwningListView = nullptr;
		bool m_Hovered = false;
		bool m_Selected = false;
	};

	class Editor;

	class EntityOutlinerView : public IWidget<EntityOutlinerView>
	{
	public:
		EntityOutlinerView(Editor* pEditor) noexcept;
		virtual ~EntityOutlinerView() noexcept override;
	private:
		NO_DISCARD float CalcDesiredWidth() const noexcept override { return 0.0f; }

		NO_DISCARD String OnDebugItemToString(const Ref<OutlinerListItem>& item) const noexcept;
		void OnEntityCreated(entity newEntity) noexcept;
		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<OutlinerListItem>& item) noexcept;

		void OnMouseEnterButton(Button* pButton) noexcept;
		void OnMouseExitButton(Button* pButton) noexcept;

		void OnMouseEnterRow(const Ref<ITableRow>& pTableRow) noexcept;
		void OnMouseExitRow(const Ref<ITableRow>& pTableRow) noexcept;

		void OnRender() noexcept override;
		NO_DISCARD const std::vector<Ref<OutlinerListItem>>* OnRequestSource() const noexcept;

		void OnSearchTextChanged(const char* pText) noexcept;
		void OnSearchTextCommitted(const char* pText, ETextCommitType commitType) noexcept;
		void OnSelectionChanged(const Ref<OutlinerListItem>& item, ESelectionType selectionType) noexcept;
		
		void OnVisibilityButtonClicked(Button* pButton) noexcept;
	private:
		std::vector<Ref<OutlinerListItem>> m_ListItems;
		Ref<ListView<Ref<OutlinerListItem>>> m_pOutlinerListView = nullptr;

		std::unordered_set<entity> m_SelectedEntities;

		Ref<VerticalBox> m_pMainBox = nullptr;
		UniquePtr<TextFilterExpressionEvaluator> m_pFilter = nullptr;

		Editor* m_pEditor = nullptr;

		bool m_SuspendNotifications = false;
	};
}