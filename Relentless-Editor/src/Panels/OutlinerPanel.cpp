#include "OutlinerPanel.h"
#include "Core/Editor.h"

#include "Subsystem/EditorSceneBridgeSubsystem.h"

#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	OutlinerPanel::OutlinerPanel() noexcept
		: PanelBase{ICON_FA_LINES_LEANING " Outliner", ImGuiWindowFlags_NoScrollbar}
	{
		Ref<VerticalBox> pVerticalBox = RLS_NEW VerticalBox();

		m_pEntityOutlinerView = pVerticalBox->AddWidget(RLS_NEW EntityOutlinerView());
		m_pEntityOutlinerView->SetVerticalSizePolicy(ESizePolicy::Stretch);
		m_pEntityOutlinerView->SetHorizontalSizePolicy(ESizePolicy::Stretch);

		SetRoot(pVerticalBox);

		SetPadding(Vector2(2.0f, 0.0f));
	}

	const Ref<EntityOutlinerView>& OutlinerPanel::GetEntityOutlinerView() const noexcept
	{
		return m_pEntityOutlinerView;
	}

	String OutlinerPanel::GetDisplayName() const noexcept
	{
		return String(ICON_FA_LINES_LEANING " Outliner");
	}

	String OutlinerPanel::GetPersistKey() const noexcept
	{
		return "Outliner";
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
			Editor::Get()->GetSubsystem<EditorSceneBridgeSubsystem>()->SetVisibilityForSelectedEntities(Keyboard::IsKeyDown(RLS_Key::LCtrl));
			return true;
		default:
			return false;
		}

		return false;
	}
}