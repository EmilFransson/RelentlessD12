#include "EntityDetailsView.h"
#include "../../../Core/Editor.h"

#include "TableRows/EntityDetailCategoryRow.h"

namespace Relentless
{
	EntityDetailsView::EntityDetailsView(Editor* aEditor) noexcept
		: m_pEditor{ aEditor }
	{
		m_pEditor->GetSelection()->OnSelectionChanged.Connect(this, &EntityDetailsView::OnSelectionChanged);

		m_pLayoutBuilder = std::make_unique<EntityDetailLayoutBuilder>(this, *m_pEditor->GetActiveScene(), *m_pEditor->GetSelection());

		std::shared_ptr<HeaderRow> pHeaderRow = std::make_shared<HeaderRow>();
		pHeaderRow->SetIsVisible(false);

		{
			Column column;
			column.pLabel = new Label("First");
			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.pLabel = new Label("Second");
			pHeaderRow->AddColumn(column);
		}

		m_pEntityDetailsTreeView = new TreeView<Ref<DetailNode>>(pHeaderRow);
		m_pEntityDetailsTreeView->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		m_pEntityDetailsTreeView->SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH);
		m_pEntityDetailsTreeView->SetClippingActive(false);

		m_pEntityDetailsTreeView
			->OnGetChildren(this, &EntityDetailsView::OnGetChildren)
			->OnRequestSource(this, &EntityDetailsView::OnRequestSource)
			->OnGenerateRow(this, &EntityDetailsView::OnGenerateRow);

		m_pMainBox = new VerticalBoxEx();

		Ref<VerticalBoxEx> pTopVerticalBox = new VerticalBoxEx();
		Ref<VerticalBoxEx> pMiddleBox = new VerticalBoxEx(Vector2(0.0f, 0.0f), true);
		pMiddleBox->SetSizePolicy(ESizePolicy::Stretch);

		Ref<VerticalBoxEx> pBottomBox = new VerticalBoxEx();
		pBottomBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Bottom);

		Ref<HorizontalBoxEx> pSearchbarBox = new HorizontalBoxEx();
		pSearchbarBox->SetSizePolicy(ESizePolicy::Stretch);
		pSearchbarBox->SetMargin(FloatRect(0.0f, 5.0f, 0.0f, 5.0f));

		pSearchbarBox->AddWidget(new SearchBar("Search..."))
			->SetSizePolicy(ESizePolicy::Stretch);

		m_pDetailsListBox = new HorizontalBoxEx(Vector2(0.0f, 0.0f), true);
		m_pDetailsListBox->SetSizePolicy(ESizePolicy::Stretch);
		m_pDetailsListBox->AddWidget(m_pEntityDetailsTreeView);

		pMiddleBox->AddWidget(m_pDetailsListBox);
		pTopVerticalBox->AddWidget(pSearchbarBox);
		m_pMainBox->AddWidget(pTopVerticalBox);
		m_pMainBox->AddWidget(pMiddleBox);

		Ref<HorizontalBoxEx> pAddComponentBox = new HorizontalBoxEx();
		pAddComponentBox->AddWidget(new Button("Add Component"))
			->SetPadding(Vector2(3.0f, 5.0f))
			->SetMargin(FloatRect(0.0f, 5.0f, 0.0f, 5.0f));

		pAddComponentBox->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);

		pBottomBox->AddWidget(pAddComponentBox);
		m_pMainBox->AddWidget(pBottomBox);
	}

	EntityDetailsView::~EntityDetailsView() noexcept
	{
		if (const auto& pSelection = m_pEditor->GetSelection())
			pSelection->OnSelectionChanged.Detach(this);
	}

	void EntityDetailsView::OnRender() noexcept
	{
		PROFILE_FUNC;
		m_pMainBox->Render();
	}

	float EntityDetailsView::CalcDesiredWidth() const noexcept
	{
		return 0.0f;
	}

	void EntityDetailsView::OnExpandCollapseButtonClicked(Button* aButton, Ref<DetailNode> aItem) noexcept
	{
		const bool isExpanded = m_pEntityDetailsTreeView->GetItemInfo(aItem).IsExpanded;
		m_pEntityDetailsTreeView->SetItemExpandedState(aItem, !isExpanded);

		m_pEntityDetailsTreeView->RequestTreeRefresh();
	}

	Ref<ITableRow> EntityDetailsView::OnGenerateRow(const Ref<DetailNode>& aItem) noexcept
	{
		const ItemInfo& info = m_pEntityDetailsTreeView->GetItemInfo(aItem);

		Ref<ITableRow> pRow = aItem->RequestRowWidget(info);

		if (EntityDetailCategoryRow* pCastRow = dynamic_cast<EntityDetailCategoryRow*>(pRow.Get()))
		{
			Button* pExpandButton = pCastRow->GetExpandButton();
			pExpandButton
				->OnClicked(std::bind(&EntityDetailsView::OnExpandCollapseButtonClicked, this, pExpandButton, aItem));
		}

		return pRow;
	}

	void EntityDetailsView::OnGetChildren(const Ref<DetailNode>& aParent, std::vector<Ref<DetailNode>>& outChildren) noexcept
	{
		outChildren = aParent->GetChildren();
	}

	const std::vector<Ref<DetailNode>>* EntityDetailsView::OnRequestSource() noexcept
	{
		//TODO: Sort... ?
		return &m_RootNodes;
	}

	void EntityDetailsView::OnSelectionChanged(entity aEntity, ESelectionState aSelectionState) noexcept
	{
		Rebuild();
		m_pEntityDetailsTreeView->RequestTreeRefresh();
	}

	void EntityDetailsView::Rebuild() noexcept
	{
		m_RootNodes = m_pLayoutBuilder->Rebuild();
	}
}