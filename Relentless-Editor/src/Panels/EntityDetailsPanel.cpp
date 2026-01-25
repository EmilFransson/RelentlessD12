#include "EntityDetailsPanel.h"

#include <UI/Views/Details/EntityDetailsView.h>

namespace Relentless
{
	EntityDetailsPanel::EntityDetailsPanel() noexcept
		: PanelBase{ ICON_FA_LINES_LEANING " Details", ImGuiWindowFlags_NoScrollbar}
	{
		m_pEntityDetailsView = new EntityDetailsView();
		SetRoot(m_pEntityDetailsView);

		SetPadding(Vector2(2.0f, 0.0f));
	}

	EntityDetailsPanel::~EntityDetailsPanel() noexcept = default;
}
