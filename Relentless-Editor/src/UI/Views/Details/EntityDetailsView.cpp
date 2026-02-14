#include "EntityDetailsView.h"

#include "Core/Editor.h"

#include "LayoutBuilders/EntityDetailLayoutBuilder.h"

#include "Subsystem/SelectionSubsystem.h"

#include "TableRows/EntityDetailCategoryRow.h"

#include "UI/Views/TreeView.h"
#include "UI/Widgets/Button.h"
#include "UI/Widgets/HorizontalBox.h"
#include "UI/Widgets/SearchBar.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	EntityDetailsView::EntityDetailsView() noexcept
	{
		Editor* pEditor = Editor::Get();

		pEditor->GetSubsystem<SelectionSubsystem>()->OnSelectionChanged.Connect(this, &EntityDetailsView::OnSelectionChanged);

		if (Scene* pScene = pEditor->GetActiveScene())
			m_pLayoutBuilder = MakeUnique<EntityDetailLayoutBuilder>(this, *pScene);

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
			column.Flags = ImGuiTableColumnFlags_NoResize;
			pHeaderRow->AddColumn(column);
		}
		{
			Column column;
			column.Weight = 0.2f;
			column.Flags = ImGuiTableColumnFlags_NoResize;
			column.pLabel = new Label("Third");
			pHeaderRow->AddColumn(column);
		}

		m_pEntityDetailsTreeView = new TreeView<Ref<DetailNode>>(pHeaderRow);
		m_pEntityDetailsTreeView->SetFont(ImGui::GetIO().Fonts->Fonts[2]);
		m_pEntityDetailsTreeView->SetFlags(ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersH | ImGuiTableFlags_SizingStretchProp);
		m_pEntityDetailsTreeView->SetClippingActive(false);

		m_pEntityDetailsTreeView
			->OnGetChildren(this, &EntityDetailsView::OnGetChildren)
			->OnRequestSource(this, &EntityDetailsView::OnRequestSource)
			->OnGenerateRow(this, &EntityDetailsView::OnGenerateRow);

		m_pMainBox = new VerticalBox();

		Ref<VerticalBox> pTopVerticalBox = new VerticalBox();
		Ref<VerticalBox> pMiddleBox = new VerticalBox(Vector2(0.0f, 0.0f), true);
		pMiddleBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		Ref<VerticalBox> pBottomBox = new VerticalBox();
		pBottomBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Bottom);

		Ref<HorizontalBox> pSearchbarBox = new HorizontalBox();
		pSearchbarBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		pSearchbarBox->SetMargin(FloatRect(0.0f, 5.0f, 0.0f, 5.0f));

		pSearchbarBox->AddWidget(new SearchBar("Search..."))
			->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		m_pDetailsListBox = new HorizontalBox(Vector2(0.0f, 0.0f), true);
		m_pDetailsListBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
		m_pDetailsListBox->AddWidget(m_pEntityDetailsTreeView);

		pMiddleBox->AddWidget(m_pDetailsListBox);
		pTopVerticalBox->AddWidget(pSearchbarBox);
		m_pMainBox->AddWidget(pTopVerticalBox);
		m_pMainBox->AddWidget(pMiddleBox);

		Ref<HorizontalBox> pAddComponentBox = new HorizontalBox();
		pAddComponentBox->AddWidget(new Button("Add Component"))
			->SetPadding(Vector2(3.0f, 5.0f))
			->SetMargin(FloatRect(0.0f, 5.0f, 0.0f, 5.0f));

		pAddComponentBox->SetHorizontalAlignmentPolicy(EHorizontalAlignmentPolicy::Center);

		pBottomBox->AddWidget(pAddComponentBox);
		m_pMainBox->AddWidget(pBottomBox);
	}

	EntityDetailsView::~EntityDetailsView() noexcept
	{
		if (const auto& pSelection = Editor::Get()->GetSubsystem<SelectionSubsystem>())
			pSelection->OnSelectionChanged.Detach(this);
	}

	void EntityDetailsView::OnRender() noexcept
	{
		PROFILE_FUNC;

		if (m_ShouldRefresh)
		{
			Rebuild();
			m_pEntityDetailsTreeView->RequestTreeRefresh();
			m_ShouldRefresh = false;
		}

		m_pMainBox->Render();
	}

	const std::vector<entity>& EntityDetailsView::GetInspectedEntities() const noexcept
	{
		return m_InspectedEntities;
	}

	uint32 EntityDetailsView::GetNumInspectedEntities() const noexcept
	{
		return static_cast<uint32>(m_InspectedEntities.size());
	}

	void EntityDetailsView::OnExpandCollapseButtonClicked(MAYBE_UNUSED Button* aButton, Ref<DetailNode> aItem) noexcept
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
			pExpandButton->OnClicked(std::bind(&EntityDetailsView::OnExpandCollapseButtonClicked, this, pExpandButton, aItem));
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

	void EntityDetailsView::OnSelectionChanged(MAYBE_UNUSED entity aEntity, MAYBE_UNUSED ESelectionState aSelectionState) noexcept
	{
		if (IsLocked())
			return;
		
		Rebuild();
		m_pEntityDetailsTreeView->RequestTreeRefresh();
	}

	void EntityDetailsView::Rebuild() noexcept
	{
		if (!m_pLayoutBuilder)
		{
			Scene* pScene = Editor::Get()->GetActiveScene();
			if (!pScene)
				return;

			m_pLayoutBuilder = MakeUnique<EntityDetailLayoutBuilder>(this, *pScene);
		}

		SelectionSubsystem* pSelection = Editor::Get()->GetSubsystem<SelectionSubsystem>();
		RLS_ASSERT(pSelection, "[EntityDetailsView::Rebuild]: Selection context is invalid.");

		m_InspectedEntities = pSelection->GetSelectedEntities();
		m_RootNodes = m_pLayoutBuilder->Rebuild();
	}
}