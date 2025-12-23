#include "DirectionalLightComponentDetailCustomization.h"

#include "../../../../Core/Selection.h"
#include "../LayoutBuilders/EntityDetailLayoutBuilder.h"
#include "../TableRows/EntityDetailRow.h"

namespace Relentless
{
	void DirectionalLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailLayoutBuilder& builder = static_cast<EntityDetailLayoutBuilder&>(aDetailLayoutBuilder);
		IDetailCategoryBuilder& categoryBuilder = builder.EditCategory("Light");

		DetailNode& typeNode = categoryBuilder.AddProperty("Type");
		typeNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestTypeRow);

		DetailNode& intensityNode = categoryBuilder.AddProperty("Intensity");
		intensityNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestIntensityRow);

		DetailNode& colorNode = categoryBuilder.AddProperty("Light Color");
		colorNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestColorRow);

		DetailNode& useTempNode = categoryBuilder.AddProperty("Use Temperature");
		useTempNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestUseTemperatureRow);
		
		DetailNode& temperatureNode = categoryBuilder.AddProperty("Temperature");
		temperatureNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestTemperatureRow);

		m_pBuilder = &builder;
	}
}
