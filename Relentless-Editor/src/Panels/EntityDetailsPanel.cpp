#include "EntityDetailsPanel.h"

#include <UI/Views/Details/EntityDetailsView.h>

#include <UI/Widgets/VerticalBox.h>
#include <UI/Widgets/WidgetSwitcher.h>

namespace Relentless
{
	EntityDetailsPanel::EntityDetailsPanel() noexcept
		: PanelBase{ ICON_FA_LINES_LEANING " Details", ImGuiWindowFlags_NoScrollbar}
	{
		SetPadding(Vector2(0.0f, 0.0f));

		Ref<VerticalBox> pRoot = RLS_NEW VerticalBox();
		pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);

		m_pEntityDetailsView = pRoot->AddWidget(RLS_NEW EntityDetailsView());

		SetRoot(pRoot);
	}

	String EntityDetailsPanel::GetDisplayName() const noexcept
	{
		return String(ICON_FA_LINES_LEANING " Details");
	}

	String EntityDetailsPanel::GetPersistKey() const noexcept
	{
		return "Details";
	}

}
