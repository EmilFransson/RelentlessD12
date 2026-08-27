#include "EditorGridDetailCustomization.h"

#include "Panels/Extensions/EditorGridExtension.h"

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/Context/ViewportDetailsContext.h"

namespace Relentless
{
	void EditorGridDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		ViewportDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<ViewportDetailsContext>();
		detailsContext.ViewportPanel->GetExtension<EditorGridExtension>()->CustomizeGridDetails(aDetailLayoutBuilder);
	}
}