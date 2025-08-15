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
		pHeaderRow->SetIsPinned(true);

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

		m_pOutlinerTreeView = new TreeView<Ref<OutlinerListItem>>(pHeaderRow);
		m_pOutlinerTreeView->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		m_pOutlinerTreeView->SetFlags(ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody |
			ImGuiTableFlags_Sortable | 
			ImGuiTableFlags_NoBordersInBodyUntilResize | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY);

		m_pOutlinerTreeView
			->OnGetChildren(this, &EntityOutlinerView::OnGetChildren)
			->OnRequestSource(this, &EntityOutlinerView::OnRequestSource)
			->OnGenerateRow(this, &EntityOutlinerView::OnGenerateRow)
			->OnDebugItemToString(this, &EntityOutlinerView::OnDebugItemToString)
			->OnSelectionChanged(this, &EntityOutlinerView::OnSelectionChanged)
			->OnContextMenuOpening(this, &EntityOutlinerView::OnContextMenuOpening);

		m_pMainBox = new VerticalBox();
		
		Ref<HorizontalBox> pHorizontalBox = new HorizontalBox();

		pHorizontalBox->Add(new SearchBar("Search...", true))
			->OnTextChanged(this, &EntityOutlinerView::OnSearchTextChanged)
			->OnTextCommitted(this, &EntityOutlinerView::OnSearchTextCommitted)
			->SetSpacing(Vector2(0.0f, 10.0f))
			->SetSizePolicy(ESizePolicy::Stretch);

		m_pOutlinerListBox = new VerticalBox(Vector2::Zero, true);
		m_pOutlinerListBox->OnFocusChanged.Connect(this, &EntityOutlinerView::OnFocusChanged);
		m_pOutlinerListBox->Add(m_pOutlinerTreeView);

		m_pMainBox->Add(pHorizontalBox);
		m_pMainBox->Add(m_pOutlinerListBox);

		m_pFilter = std::make_unique<TextFilterExpressionEvaluator>();
	}

	EntityOutlinerView::~EntityOutlinerView() noexcept
	{
		if (Scene* pScene = m_pEditor->GetActiveScene())
			pScene->OnEntityCreated.Detach(this);
	}

	Ref<ContextMenu> EntityOutlinerView::OnContextMenuOpening(const Ref<OutlinerListItem>& item) noexcept
	{
		Ref<MenuBuilder> pBuilder = new MenuBuilder();

		Ref<ContextMenu> pMenu = pBuilder
			->AddSection("ENTITY OPTIONS")
				->BeginSubMenu("Edit", "Export stuff", ICON_FA_PEN_RULER)
					->AddMenuEntry("Duplicate", "Duplicate Selection", ICON_FA_COPY, this, &EntityOutlinerView::OnDelete)
					->AddMenuEntry("Delete", "Delete Current Selection", ICON_FA_DELETE_LEFT, this, &EntityOutlinerView::OnDelete)
					->AddMenuEntry("Rename", "Rename Current Selection", ICON_FA_PEN, this, &EntityOutlinerView::OnDelete)
						->BeginSubMenu("TEST", "TESTING", ICON_FA_FOLDER)
							->AddMenuEntry("Entry", "An Entry", ICON_FA_PEN, this, &EntityOutlinerView::OnDelete)
						->EndSubMenu()
				->EndSubMenu()
			->AddMenuEntry("Delete", "Delete the item!", ICON_FA_DELETE_LEFT, this, &EntityOutlinerView::OnDelete)
			->AddMenuEntry("Rename", "Rename the entity", ICON_FA_PEN, this, &EntityOutlinerView::OnDelete)
			->AddSection("VIEW OPTIONS")
				->BeginSubMenu("Export", "Export stuff", ICON_FA_EXPAND)
					->AddMenuEntry("DO SOMETHING", "DO!", ICON_FA_DELETE_LEFT, this, &EntityOutlinerView::OnDelete)
				->EndSubMenu()
			->Build();

		pMenu->SetSpacing(Vector2(8.0f, 1.0f));
		return pMenu;
	}

	void EntityOutlinerView::OnDelete() noexcept
	{
		RLS_CORE_INFO("OH HI MARK!!!");
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

	void EntityOutlinerView::OnFocusChanged(bool focused) noexcept
	{
		m_pOutlinerTreeView->TriggerFocusChange(focused);
	}

	Ref<ITableRow> EntityOutlinerView::OnGenerateRow(const Ref<OutlinerListItem>& item) noexcept
	{
		Ref<OutlinerTableRow> pRow = new OutlinerTableRow(m_pOutlinerTreeView);
		pRow->OnMouseEnter(this, &EntityOutlinerView::OnMouseEnterRow);
		pRow->OnMouseExit(this, &EntityOutlinerView::OnMouseExitRow);

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

		const bool highlighted = m_pOutlinerTreeView->IsItemHighlighted(item);

		HorizontalBox* pDisplayBox = pRow->SetWidget(new HorizontalBox(), 1);

		pDisplayBox->Add(new Label(icon, ImGui::GetIO().Fonts->Fonts[2]))
			->SetTooltipText(name)
			->SetAlpha(0.8f);

		Label* pDisplayNameLabel = pDisplayBox->Add(new Label(name, ImGui::GetIO().Fonts->Fonts[2]));
		pDisplayNameLabel->SetTooltipText(name);
		
		if (highlighted && m_pFilter->TestTextFilter(name, ETextFilterTextComparisonMode::Partial))
			pDisplayNameLabel->SetHighlightedSubstring(m_pFilter->GetFilterText());

		Label* pTypeLabel = pRow->SetWidget(new Label(type, ImGui::GetIO().Fonts->Fonts[2]), 2);
		pTypeLabel->SetTooltipText(type);
		pTypeLabel->SetAlpha(0.7f);

		if (highlighted && m_pFilter->TestTextFilter(type, ETextFilterTextComparisonMode::Partial))
			pTypeLabel->SetHighlightedSubstring(m_pFilter->GetFilterText());

		return pRow;
	}

	void EntityOutlinerView::OnGetChildren(const Ref<OutlinerListItem>& pParent, std::vector<Ref<OutlinerListItem>>& outChildren) noexcept
	{

	}

	void EntityOutlinerView::OnMouseEnterButton(Button* pButton) noexcept
	{
		pButton->SetAlpha(1.0f);
	}

	void EntityOutlinerView::OnMouseExitButton(Button* pButton) noexcept
	{
		pButton->SetAlpha(0.7f);
	}

	void EntityOutlinerView::OnMouseEnterRow(ITableRow* pTableRow) noexcept
	{
		static_cast<Button*>(static_cast<OutlinerTableRow*>(pTableRow)->GetWidget(0).Get())->SetIsVisible(true);
	}

	void EntityOutlinerView::OnMouseExitRow(ITableRow* pTableRow) noexcept
	{
		Button* pButton = static_cast<Button*>(static_cast<OutlinerTableRow*>(pTableRow)->GetWidget(0).Get());

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

		m_pOutlinerTreeView->ClearSelection();
		m_pOutlinerTreeView->ClearHightlightedItems();
		m_pOutlinerTreeView->ClearItemsSource();

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
						m_pOutlinerTreeView->SetItemSelection(pEntityItem, ESelectionType::Selected);

					m_pOutlinerTreeView->SetItemHighlighted(pEntityItem);
					m_ListItems.emplace_back(std::move(pEntityItem));
				}
			});

		m_SuspendNotifications = false;
	}

	void EntityOutlinerView::OnSearchTextCommitted(const char* pText, ETextCommitType commitType) noexcept
	{
		if (commitType != ETextCommitType::OnEnter)
			return;

		m_pOutlinerTreeView->ClearSelection();
		m_SelectedEntities.clear();

		for (size_t i = 0u; i < m_ListItems.size(); ++i)
		{
			if (!m_pOutlinerTreeView->IsItemSelected(m_ListItems[i]))
				m_pOutlinerTreeView->SetItemSelection(m_ListItems[i], ESelectionType::Selected);
		}
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