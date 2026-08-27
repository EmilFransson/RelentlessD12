#include "ViewportRenderDetailCustomization.h"

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/Context/ViewportDetailsContext.h"

namespace Relentless
{
	void ViewportRenderDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		ViewportDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<ViewportDetailsContext>();
		IDetailCategoryBuilder& renderingCategoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_PAINTBRUSH "  Rendering");
		
		renderingCategoryBuilder.AddProperty<int>("MSAA",
			[pClient = &detailsContext.ViewportPanel->GetClient()]() { return static_cast<int>(Math::Log2f(static_cast<float>(pClient->GetRenderQualitySettings().MSAASampleCount))); },
			[pClient = &detailsContext.ViewportPanel->GetClient()](const int& aValue) { pClient->SetMSAASamples(static_cast<EMSAASampleCount>(static_cast<int>(Math::Pow2f(aValue)))); },
			0)
			.NameSlot().Label("MSAA")
			.ValueSlot().ComboBox().Options({ "Off", "2x", "4x", "8x" });
	}
}
