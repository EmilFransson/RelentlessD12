#pragma once

#include "UI/Widgets/ContextMenu.h"
#include "UI/Widgets/ITableRow.h"

namespace Relentless
{
	class Button;

	class DetailCategoryRow : public ITableRow
	{
	public:
		DetailCategoryRow(StringView aName, bool aIsExpanded) noexcept;
		virtual ~DetailCategoryRow() noexcept override = default;

		NO_DISCARD Button* GetExpandButton() const noexcept;

		template<typename Func>
		DetailCategoryRow& OnContextMenuOpening(Func&& aCallback);

		template<typename InstanceType>
		DetailCategoryRow& OnContextMenuOpening(InstanceType* aInstanceType, Ref<ContextMenu>(InstanceType::*aMethod)());
		
		NO_DISCARD Vector2 ReportSize() const noexcept override;
	protected:
		const Color& GetBackgroundColor() const noexcept override;
		uint32 GetNumColumns() noexcept override;

		void OnRenderColumn(uint32 aColumn) noexcept override;
	private:
		std::vector<Ref<IBaseWidget>> m_ColumnWidgets2;
		Callback<Ref<ContextMenu>()> m_OnContextMenuOpening;
	};

	template<typename Func>
	DetailCategoryRow& DetailCategoryRow::OnContextMenuOpening(Func&& aCallback)
	{
		m_OnContextMenuOpening = Callback<Ref<ContextMenu>()>(std::forward<Func>(aCallback));
		return *this;
	}

	template<typename InstanceType>
	DetailCategoryRow& DetailCategoryRow::OnContextMenuOpening(InstanceType* aInstanceType, Ref<ContextMenu>(InstanceType::*aMethod)())
	{
		m_OnContextMenuOpening = [aInstanceType, aMethod]() { return (aInstanceType->*aMethod()); };
		return *this;
	}
}