#include "OutlinerPanel.h"
#include "../Core/Editor.h"
#include "../UI/Views/Outliner/EntityOutlinerView.h"

namespace Relentless
{
	OutlinerPanel::OutlinerPanel() noexcept
		: PanelBase{ICON_FA_LINES_LEANING " Outliner", ImGuiWindowFlags_NoScrollbar}
	{
		m_pEntityOutlinerView = new EntityOutlinerView();
		SetRoot(m_pEntityOutlinerView);
	}

	const Ref<EntityOutlinerView>& OutlinerPanel::GetEntityOutlinerView() const noexcept
	{
		return m_pEntityOutlinerView;
	}

	bool OutlinerPanel::OnKeyPressedEvent(KeyPressedEvent& aEvent) noexcept
	{
		switch (aEvent.key)
		{
		case RLS_Key::Delete:
			m_pEntityOutlinerView->OnDeleteSelection();
			return true;
		case RLS_Key::F2:
			m_pEntityOutlinerView->OnRenameSelection();
			return true;
		case RLS_Key::D:
		{
			if (Keyboard::IsKeyDown(RLS_Key::LCtrl))
			{
				m_pEntityOutlinerView->DuplicateSelection();
				return true;
			}
			break;
		}
		case RLS_Key::H:
			Editor::Get()->SetVisibilityForSelectedEntities(Keyboard::IsKeyDown(RLS_Key::LCtrl));
			return true;
		}

		return false;
	}
}