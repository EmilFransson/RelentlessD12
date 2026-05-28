#pragma once
#include "UI/Widgets/IStylableWidget.h"

namespace Relentless
{
	class ContextMenu;
	class ContextMenuBuilder;
	class HorizontalBox;

	class SubMenuRow : public IStylableWidget<SubMenuRow>
	{
	public:
		SubMenuRow(StringView aText) noexcept;

		SubMenuRow* OnOpen(Callback<void(ContextMenuBuilder&)>&& aCallback) noexcept;
		virtual void OnRender() noexcept override;
	protected:
		
		NO_DISCARD Vector2 ReportSize() const noexcept override;
	private:
		String m_Text;
		Callback<void(ContextMenuBuilder&)> m_OnOpenCallback;
		Ref<ContextMenu> m_pSubMenu;
		bool m_Enabled = true;
	};
}