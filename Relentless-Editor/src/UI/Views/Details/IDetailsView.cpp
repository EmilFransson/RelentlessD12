#include "IDetailsView.h"

#include "UI/Views/TreeView.h"

#include "UI/Widgets/Button.h"

namespace Relentless
{
	IDetailsView::IDetailsView() noexcept
	{
		m_pDetailsTreeView = BuildTreeView();
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
		pTreeView->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		pTreeView->SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchProp);
		pTreeView->SetClippingActive(false);

		pTreeView
			->OnGetChildren(this, &IDetailsView::OnGetChildren)
			->OnRequestSource(this, &IDetailsView::OnRequestSource)
			->OnGenerateRow(this, &IDetailsView::OnGenerateRow);

		return pTreeView;
	}

	bool IDetailsView::IsLocked() const noexcept
	{
		return m_IsLocked;
	}

	void IDetailsView::OnRender() noexcept
	{
		m_pDetailsTreeView->Render();
	}

	void IDetailsView::RequestRefresh() noexcept
	{
		m_ShouldRefresh = true;
	}

	void IDetailsView::SetContext(void* aContext) noexcept
	{
		m_pContext = aContext;
	}

	void IDetailsView::OnExpandCollapseButtonClicked(MAYBE_UNUSED Button* aButton, Ref<DetailNode> aItem) noexcept
	{
		const bool isExpanded = m_pDetailsTreeView->GetItemInfo(aItem).IsExpanded;
		m_pDetailsTreeView->SetItemExpandedState(aItem, !isExpanded);

		m_pDetailsTreeView->RequestTreeRefresh();
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

		return pRow;
	}

	void IDetailsView::OnGetChildren(const Ref<DetailNode>& aParent, std::vector<Ref<DetailNode>>& outChildren) noexcept
	{
		outChildren = aParent->GetChildren();
	}

	const std::vector<Ref<DetailNode>>* IDetailsView::OnRequestSource() noexcept
	{
		return &m_RootNodes;
	}

}