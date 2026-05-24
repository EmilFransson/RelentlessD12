#include "IDetailsView.h"

#include "UI/Views/TreeView.h"

#include "UI/Widgets/Button.h"

namespace Relentless
{
	IDetailsView::IDetailsView() noexcept
	{
		m_pMainBox = RLS_NEW VerticalBox();
		m_pMainBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pMainBox->SetVerticalSizePolicy(ESizePolicy::Stretch);

		m_pHeaderBox = m_pMainBox->AddWidget(RLS_NEW VerticalBox());
		m_pDetailsTreeView = m_pMainBox->AddWidget(BuildTreeView());
	}

	Ref<TreeView<Ref<DetailNode>>> IDetailsView::BuildTreeView() noexcept
	{
		std::shared_ptr<HeaderRow> pHeaderRow = std::make_shared<HeaderRow>();
		pHeaderRow->SetIsVisible(false);

		{
			Column column;
			column.Weight = 30.0f;
			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.Weight = 30.0f;
			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.Weight = 27.0f;
			column.Flags = ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoResize;
			pHeaderRow->AddColumn(column);
		}

		Ref<TreeView<Ref<DetailNode>>> pTreeView = RLS_NEW TreeView<Ref<DetailNode>>(pHeaderRow);
		pTreeView->SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchProp);
		pTreeView->SetClippingActive(false);
		pTreeView->SetSpacing(Vector2::Zero);
		pTreeView->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pTreeView->SetVerticalSizePolicy(ESizePolicy::Stretch);

		pTreeView
			->OnGetChildren(this, &IDetailsView::OnGetChildren)
			->OnRequestSource(this, &IDetailsView::OnRequestSource)
			->OnGenerateRow(this, &IDetailsView::OnGenerateRow);

		return pTreeView;
	}

	VerticalBox* IDetailsView::GetHeader() const noexcept
	{
		return m_pHeaderBox;
	}

	void IDetailsView::OnRender() noexcept
	{
		m_pMainBox->AssignSize(GetAssignedSize());
		m_pMainBox->Render();
	}

	void IDetailsView::RequestRefresh() noexcept
	{
		m_pDetailsTreeView->RequestTreeRefresh();
		m_ManualRefreshTriggered = true;
	}

	void IDetailsView::SetContext(void* aContext) noexcept
	{
		m_pContext = aContext;
	}

	void IDetailsView::OnExpandCollapseButtonClicked(MAYBE_UNUSED Button* aButton, Ref<DetailNode> aItem) noexcept
	{
		const bool isExpanded = m_pDetailsTreeView->GetItemInfo(aItem).IsExpanded;
		m_pDetailsTreeView->SetItemExpandedState(aItem, !isExpanded);
	}

	Ref<ITableRow> IDetailsView::OnGenerateRow(const Ref<DetailNode>& aItem) noexcept
	{
		const ItemInfo& info = m_pDetailsTreeView->GetItemInfo(aItem);

		Ref<ITableRow> pRow = aItem->RequestRowWidget(info);

		if (DetailCategoryRow* pCastRow = dynamic_cast<DetailCategoryRow*>(pRow.Get()))
		{
			Button* pExpandButton = pCastRow->GetExpandButton();
			pExpandButton->OnClicked(std::bind(&IDetailsView::OnExpandCollapseButtonClicked, this, pExpandButton, aItem));
		}
		else if (DetailGroupRow* pCastRow = dynamic_cast<DetailGroupRow*>(pRow.Get()))
		{
			Button* pExpandButton = pCastRow->GetExpandButton();
			pExpandButton->OnClicked(std::bind(&IDetailsView::OnExpandCollapseButtonClicked, this, pExpandButton, aItem));
		}

		return pRow;
	}

	void IDetailsView::OnGetChildren(const Ref<DetailNode>& aParent, std::vector<Ref<DetailNode>>& outChildren) noexcept
	{
		outChildren = aParent->GetChildren();
	}

	const std::vector<Ref<DetailNode>>* IDetailsView::OnRequestSource() noexcept
	{
		OnPreRequestSource(m_ManualRefreshTriggered);
		m_ManualRefreshTriggered = false;

		return &m_RootNodes;
	}

}