#pragma once
#include <Relentless.h>
#include "Customizations/IDetailCustomization.h"
#include "DetailNode.h"
#include "IDetailsView.h"

namespace Relentless
{
	class Editor;
	class EntityDetailLayoutBuilder;
	enum class ESelectionState : uint8;

	class EntityDetailsView : public IDetailsView
	{
	public:
		explicit EntityDetailsView() noexcept;
		virtual ~EntityDetailsView() noexcept override;

		NO_DISCARD float CalcDesiredWidth() const noexcept override;

		NO_DISCARD const std::vector<entity>& GetInspectedEntities() const noexcept;
		NO_DISCARD uint32 GetNumInspectedEntities() const noexcept;

		void OnRender() noexcept override;
	private:
		void OnExpandCollapseButtonClicked(Button* aButton, Ref<DetailNode> aItem) noexcept;
		
		NO_DISCARD Ref<ITableRow> OnGenerateRow(const Ref<DetailNode>& aItem) noexcept;
		void OnGetChildren(const Ref<DetailNode>& aParent, std::vector<Ref<DetailNode>>& outChildren) noexcept;
		NO_DISCARD const std::vector<Ref<DetailNode>>* OnRequestSource() noexcept;
		void OnSelectionChanged(MAYBE_UNUSED entity aEntity, MAYBE_UNUSED ESelectionState aSelectionState) noexcept;

		void Rebuild() noexcept;
	private:
		std::vector<Ref<DetailNode>> m_RootNodes;
		std::vector<entity> m_InspectedEntities;

		Ref<TreeView<Ref<DetailNode>>> m_pEntityDetailsTreeView = nullptr;
		UniquePtr<EntityDetailLayoutBuilder> m_pLayoutBuilder = nullptr;
		Ref<VerticalBoxEx> m_pMainBox = nullptr;
		Ref<HorizontalBoxEx> m_pDetailsListBox = nullptr;
	};
}