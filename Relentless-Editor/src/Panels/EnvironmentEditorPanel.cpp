#include "EnvironmentEditorPanel.h"

#include "UI/Views/Details/EnvironmentDetailsView.h"
#include "UI/Widgets/VerticalBox.h"

namespace Relentless
{
	EnvironmentEditorPanel::EnvironmentEditorPanel(const std::vector<Ref<Environment>>& someEnvironments) noexcept
		:PanelBase("Environment Editor", ImGuiWindowFlags_None)
	{
		SetPadding(Vector2(0.0f, 0.0f));

		Ref<VerticalBox> pRoot = RLS_NEW VerticalBox();
		pRoot->SetVerticalSizePolicy(ESizePolicy::Stretch);

		pRoot->AddWidget(RLS_NEW EnvironmentDetailsView(someEnvironments));

		SetRoot(pRoot);
	}
}
