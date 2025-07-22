#include "EntityOutlinerView.h"

#include "../../../Core/Editor.h"

namespace Relentless
{
	EntityOutlinerView::EntityOutlinerView(Editor* pEditor) noexcept
		: m_pEditor{pEditor}
	{
		if (Scene* pScene = m_pEditor->GetActiveScene())
			pScene->OnEntityCreated.Connect(this, &EntityOutlinerView::OnEntityCreated);

		std::shared_ptr<HeaderRow> pHeaderRow = std::make_shared<HeaderRow>();

		{
			Column column;
			column.pLabel = new Label(ICON_FA_EYE);
			column.pLabel
				->SetAlpha(0.7f)
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetTooltipText("Visibility");

			column.Flags = ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;
			column.Weight = 22.0f;

			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.pLabel = new Label("Item Label");
			column.pLabel
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetTooltipText("Item Label");

			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.pLabel = new Label("Type");
			column.pLabel
				->SetFont(ImGui::GetIO().Fonts->Fonts[2])
				->SetTooltipText("Type");

			pHeaderRow->AddColumn(column);
		}

		m_pOutlinerListView = new ListView<Ref<OutlinerListItem>>(pHeaderRow);
		m_pOutlinerListView->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		m_pOutlinerListView->SetFlags(ImGuiTableFlags_Borders | ImGuiTableFlags_Sortable | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_Resizable);

		m_pOutlinerListView
			->OnRequestSource(this, &EntityOutlinerView::OnRequestSource)
			->OnGenerateRow(this, &EntityOutlinerView::OnGenerateRow)
			->OnDebugItemToString(this, &EntityOutlinerView::OnDebugItemToString)
			->OnSelectionChanged(this, &EntityOutlinerView::OnSelectionChanged)
			->OnMouseEnterRow(this, &EntityOutlinerView::OnMouseEnterRow)
			->OnMouseExitRow(this, &EntityOutlinerView::OnMouseExitRow);

		m_pMainBox = new VerticalBox();

		Ref<HorizontalBox> pHorizontalBox = new HorizontalBox();
		pHorizontalBox->Add(new SearchBar("Search...", true))
			->OnTextChanged(this, &EntityOutlinerView::OnSearchTextChanged)
			->OnTextCommitted(this, &EntityOutlinerView::OnSearchTextCommitted)
			->SetSpacing(Vector2(0.0f, 10.0f))
			->SetSizePolicy(ESizePolicy::Stretch);

		m_pMainBox->Add(pHorizontalBox);
		m_pMainBox->Add(m_pOutlinerListView);

		m_pFilter = std::make_unique<TextFilterExpressionEvaluator>();
	}

	EntityOutlinerView::~EntityOutlinerView() noexcept
	{
		if (Scene* pScene = m_pEditor->GetActiveScene())
			pScene->OnEntityCreated.Detach(this);
	}

	String EntityOutlinerView::OnDebugItemToString(const Ref<OutlinerListItem>& item) const noexcept
	{
		if (item->IsEntityItem())
			return std::format("Entity: {}", item->Entity);
		else
			return std::format("Filter: {}", item->pFilter->GetName());
	}

	void EntityOutlinerView::OnEntityCreated(entity newEntity) noexcept
	{
		Ref<OutlinerListItem> pEntityItem = new OutlinerListItem();
		pEntityItem->Entity = newEntity;
		m_ListItems.emplace_back(std::move(pEntityItem));
	}

	Ref<ITableRow> EntityOutlinerView::OnGenerateRow(const Ref<OutlinerListItem>& item) noexcept
	{
		Ref<OutlinerTableRow> pRow = new OutlinerTableRow(m_pOutlinerListView);
		pRow->SetMargin(FloatRect(1.0f, 0.0f, 0.0f, 0.0f), 0);

		const bool isEntityItem = item->IsEntityItem();

		const String icon = isEntityItem ? ICON_FA_CUBE : ICON_FA_FOLDER;
		const String name = isEntityItem ? m_pEditor->GetActiveScene()->GetEntityManager().Get<NameComponent>(item->Entity).Name : item->pFilter->GetName();
		const String type = isEntityItem ? "Entity" : "Filter";

		Button* pButton = pRow->SetWidget(new Button(ICON_FA_EYE), 0);
		pButton
			->OnClicked(std::bind(&EntityOutlinerView::OnVisibilityButtonClicked, this, pButton))
			->OnMouseEnter(std::bind(&EntityOutlinerView::OnMouseEnterButton, this, pButton))
			->OnMouseExit(std::bind(&EntityOutlinerView::OnMouseExitButton, this, pButton))
			->SetBackgroundColor(Colors::Transparent)
			->SetActiveColor(Colors::Transparent)
			->SetHoverColor(Colors::Transparent)
			->SetBorderColor(Colors::Transparent)
			->SetFont(ImGui::GetIO().Fonts->Fonts[2])
			->SetAlpha(0.7f)
			->SetTooltipText("Toggles the visibility of this item")
			->SetIsVisible(false);

		const bool highlighted = m_pOutlinerListView->IsItemHighlighted(item);

		HorizontalBox* pDisplayBox = pRow->SetWidget(new HorizontalBox(), 1);

		pDisplayBox->Add(new Label(icon, ImGui::GetIO().Fonts->Fonts[2]))
			->SetTooltipText(name)
			->SetAlpha(0.8f);

		Label* pDisplayNameLabel = pDisplayBox->Add(new Label(name, ImGui::GetIO().Fonts->Fonts[2]));
		pDisplayNameLabel->SetTooltipText(name);
		
		if (m_pFilter->TestTextFilter(name, ETextFilterTextComparisonMode::Partial))
			pDisplayNameLabel->SetHighlightedSubstring(m_pFilter->GetFilterText());

		Label* pTypeLabel = pRow->SetWidget(new Label(type, ImGui::GetIO().Fonts->Fonts[2]), 2);
		pTypeLabel->SetTooltipText(type);
		pTypeLabel->SetAlpha(0.7f);

		if (m_pFilter->TestTextFilter(type, ETextFilterTextComparisonMode::Partial))
			pTypeLabel->SetHighlightedSubstring(m_pFilter->GetFilterText());

		return pRow;
	}

	void EntityOutlinerView::OnMouseEnterButton(Button* pButton) noexcept
	{
		pButton->SetAlpha(1.0f);
	}

	void EntityOutlinerView::OnMouseExitButton(Button* pButton) noexcept
	{
		pButton->SetAlpha(0.7f);
	}

	void EntityOutlinerView::OnMouseEnterRow(const Ref<ITableRow>& pTableRow) noexcept
	{
		OutlinerTableRow* pRow = static_cast<OutlinerTableRow*>(pTableRow.Get());
		static_cast<Button*>(pRow->GetWidget(0).Get())->SetIsVisible(true);
	}

	void EntityOutlinerView::OnMouseExitRow(const Ref<ITableRow>& pTableRow) noexcept
	{
		OutlinerTableRow* pRow = static_cast<OutlinerTableRow*>(pTableRow.Get());
		Button* pButton = static_cast<Button*>(pRow->GetWidget(0).Get());

		if (pButton->GetText() == ICON_FA_EYE)
			pButton->SetIsVisible(false);
	}

	void EntityOutlinerView::OnRender() noexcept
	{
		m_pMainBox->Render();
	}

	const std::vector<Ref<OutlinerListItem>>* EntityOutlinerView::OnRequestSource() const noexcept
	{
		return &m_ListItems;
	}

	void EntityOutlinerView::OnSearchTextChanged(const char* pText) noexcept
	{
		m_SuspendNotifications = true;

		m_pOutlinerListView->ClearSelection();
		m_pOutlinerListView->ClearHightlightedItems();
		m_pOutlinerListView->ClearItemsSource();

		m_ListItems.clear();
		
		m_pFilter->SetFilterText(pText);

		const bool containsEntityLabel = m_pFilter->TestTextFilter(OutlinerListItem::GetEntityTypeAsString(), ETextFilterTextComparisonMode::Partial);

		m_pEditor->GetActiveScene()->GetEntityManager().Collect<NameComponent>().Do([&](entity e, const NameComponent& nc)
			{
				if (m_pFilter->TestTextFilter(nc.Name, ETextFilterTextComparisonMode::Partial) || containsEntityLabel)
				{
					Ref<OutlinerListItem> pEntityItem = new OutlinerListItem();
					pEntityItem->Entity = e;

					if(m_SelectedEntities.contains(e))
						m_pOutlinerListView->SetItemSelection(pEntityItem, ESelectionType::Selected);

					m_pOutlinerListView->SetItemHighlighted(pEntityItem);
					m_ListItems.emplace_back(std::move(pEntityItem));
				}
			});

		m_SuspendNotifications = false;
	}

	void EntityOutlinerView::OnSearchTextCommitted(const char* pText, ETextCommitType commitType) noexcept
	{
		if (commitType != ETextCommitType::OnEnter)
			return;

		for (size_t i = 0u; i < m_ListItems.size(); ++i)
		{
			if (!m_pOutlinerListView->IsItemSelected(m_ListItems[i]))
				m_pOutlinerListView->SetItemSelection(m_ListItems[i], ESelectionType::Selected);
		}

		//TODO: Act on OnCleared! :D
	}

	void EntityOutlinerView::OnSelectionChanged(const Ref<OutlinerListItem>& item, ESelectionType selectionType) noexcept
	{
		if (m_SuspendNotifications)
			return;

		if (selectionType == ESelectionType::Selected)
		{
			if (item->IsEntityItem())
				m_SelectedEntities.insert(item->Entity);
		}
		else
		{
			if (item->IsEntityItem())
				m_SelectedEntities.erase(item->Entity);
		}
	}

	void EntityOutlinerView::OnVisibilityButtonClicked(Button* pButton) noexcept
	{
		if (pButton->GetText() == ICON_FA_EYE)
			pButton->SetText(ICON_FA_EYE_SLASH);
		else
			pButton->SetText(ICON_FA_EYE);
	}
}