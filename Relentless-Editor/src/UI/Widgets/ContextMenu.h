#pragma once
#include "IStylableWidget.h"

#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	class ContextMenu : public IStylableWidget<ContextMenu>
	{
	public:
		ContextMenu(bool aIsSubMenu = false) noexcept;
		virtual ~ContextMenu() noexcept;

		void AddRows(const std::vector<Ref<IBaseWidget>>& someRows) noexcept;

		void OnRender() noexcept override;

		Broadcaster<void()> OnClosed;

	protected:
		NO_DISCARD Vector2 ReportSize() const noexcept override;
	private:
		void RenderRows() noexcept;
	private:
		Ref<VerticalBox> m_pRoot;
		bool m_IsOpen = false;
		bool m_IsSubMenu = false;
	};
}