#include "EnvironmentDetailCustomization.h"

#include "UI/Views/Details/Context/EnvironmentDetailsContext.h"
#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/DetailPropertyRowBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"

namespace Relentless
{
	void EnvironmentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EnvironmentDetailsContext& detailsContext = aDetailLayoutBuilder.GetDetailsView()->GetContext<EnvironmentDetailsContext>();
		const bool multiSelection = detailsContext.Environments.size() > 1u;

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_CAMERA "  Camera");

		Ref<MultiPropertyHandle<float>> pIntensityHandle = RLS_NEW MultiPropertyHandle<float>(
			[&detailsContext](uint32 aIndex) { return detailsContext.Environments[aIndex]->GetIntensity(); },
			[&detailsContext](const float& aValue, uint32 aIndex) { detailsContext.Environments[aIndex]->SetIntensity(aValue); },
			detailsContext.Environments.size(),
			1.0f
		);

		categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle)
			.NameSlot().Label("Intensity")
			.ValueSlot().Slider().Range(0.0f, 1.0f);
	}

	bool EnvironmentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		return !aDetailLayoutBuilder.GetDetailsView()->GetContext<EnvironmentDetailsContext>().Environments.empty();
	}
}