#pragma once
#include "UI/Widgets/IWidget.h"

#include "DetailNode.h"

#include "LayoutBuilders/IDetailLayoutBuilder.h"

namespace Relentless
{
	class Button;
	template<typename T> class TreeView;

	class IDetailsView : public IWidget<IDetailsView>
	{
	public:
		IDetailsView() noexcept;
		virtual ~IDetailsView() noexcept = default;

		virtual Ref<TreeView<Ref<DetailNode>>> BuildTreeView() noexcept;

		template<typename ContextType>
		NO_DISCARD ContextType& GetContext() noexcept;

		NO_DISCARD bool IsLocked() const noexcept;

		virtual void OnRender() noexcept override;
		
		template<typename InspectedType>
		void Rebuild() noexcept;

		void RequestRefresh() noexcept;

		void SetContext(void* aContext) noexcept;
	private:
		virtual void OnExpandCollapseButtonClicked(MAYBE_UNUSED Button* aButton, Ref<DetailNode> aItem) noexcept;
		NO_DISCARD virtual Ref<ITableRow> OnGenerateRow(const Ref<DetailNode>& aItem) noexcept;
		virtual void OnGetChildren(const Ref<DetailNode>& aParent, std::vector<Ref<DetailNode>>& outChildren) noexcept;
		NO_DISCARD virtual const std::vector<Ref<DetailNode>>* OnRequestSource() noexcept;
	protected:
		std::vector<Ref<DetailNode>> m_RootNodes;
		Ref<TreeView<Ref<DetailNode>>> m_pDetailsTreeView = nullptr;
		bool m_ShouldRefresh = true;
	private:
		bool m_IsLocked = false;
		void* m_pContext = nullptr;
	};

	template<typename ContextType>
	ContextType& IDetailsView::GetContext() noexcept
	{
		RLS_ASSERT(m_pContext, "[IDetailsView::GetContext]: Context Is invalid.");
		return *static_cast<ContextType*>(m_pContext);
	}

	template<typename InspectedType>
	void IDetailsView::Rebuild() noexcept
	{
		IDetailLayoutBuilder layoutBuilder(this);
		m_RootNodes = layoutBuilder.Build<InspectedType>();
	}

}