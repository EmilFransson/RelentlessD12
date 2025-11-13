#pragma once
#include <Relentless.h>
#include "Customizations/IDetailCustomization.h"
#include "DetailNode.h"
#include "IDetailsView.h"
#include "LayoutBuilders/EntityDetailLayoutBuilder.h"

namespace Relentless
{
	class Editor;

	class EntityDetailsView : public IDetailsView
	{
	public:
		explicit EntityDetailsView(Editor* aEditor) noexcept;
		virtual ~EntityDetailsView() noexcept override;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;

		void OnRender() noexcept override;
	private:
		void OnExpandCollapseButtonClicked(Button* aButton, Ref<DetailNode> aItem) noexcept;
		
		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<DetailNode>& aItem) noexcept;
		void OnGetChildren(const Ref<DetailNode>& aParent, std::vector<Ref<DetailNode>>& outChildren) noexcept;
		NO_DISCARD const std::vector<Ref<DetailNode>>* OnRequestSource() noexcept;
		void OnSelectionChanged(entity aEntity, ESelectionState aSelectionState) noexcept;

		void Rebuild() noexcept;
	private:
		std::vector<Ref<DetailNode>> m_RootNodes;

		Ref<TreeView<Ref<DetailNode>>> m_pEntityDetailsTreeView = nullptr;
		UniquePtr<EntityDetailLayoutBuilder> m_pLayoutBuilder = nullptr;
		Ref<VerticalBoxEx> m_pMainBox = nullptr;
		Ref<HorizontalBoxEx> m_pDetailsListBox = nullptr;
		Editor* m_pEditor = nullptr;
	};
}