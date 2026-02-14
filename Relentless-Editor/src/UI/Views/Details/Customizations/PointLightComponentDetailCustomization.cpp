#include "PointLightComponentDetailCustomization.h"

#include "UI/Views/Details/LayoutBuilders/EntityDetailLayoutBuilder.h"
#include "UI/Views/Details/TableRows/EntityDetailRow.h"

namespace Relentless
{
	void PointLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailLayoutBuilder& builder = static_cast<EntityDetailLayoutBuilder&>(aDetailLayoutBuilder);
		IDetailCategoryBuilder& categoryBuilder = builder.EditCategory("Light");

		DetailNode& typeNode = categoryBuilder.AddProperty("Type");
		typeNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestTypeRow);

		DetailNode& intensityNode = categoryBuilder.AddProperty("Intensity");
		intensityNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestIntensityRow);

		DetailNode& intensityUnitsNode = categoryBuilder.AddProperty("Intensity Units");
		intensityUnitsNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestIntensityUnitsRow);

		DetailNode& colorNode = categoryBuilder.AddProperty("Light Color");
		colorNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestColorRow);

		DetailNode& attenuationRadiusNode = categoryBuilder.AddProperty("Attenuation Radius");
		attenuationRadiusNode.OnRequestRow(this, &PointLightComponentDetailCustomization::OnRequestAttenuationRadiusRow);

		DetailNode& useTempNode = categoryBuilder.AddProperty("Use Temperature");
		useTempNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestUseTemperatureRow);

		DetailNode& temperatureNode = categoryBuilder.AddProperty("Temperature");
		temperatureNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestTemperatureRow);

		m_pBuilder = &builder;
	}

	float PointLightComponentDetailCustomization::GetAttenuationRadius() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<PointLightComponent>(GetPrimaryEntity()).GetAttenuationRadius();
	}

	bool PointLightComponentDetailCustomization::IsAttenuationRadiusDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [&entityManager](entity aEntity)
			{
				auto& plc = entityManager.Get<PointLightComponent>(aEntity);
				return Math::AreValuesClose(plc.GetAttenuationRadius(), DEFAULT_ATTENUATION_RADIUS);
			});
	}

	void PointLightComponentDetailCustomization::OnAttenuationRadiusChanged(float aRadius) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity) {	entityManager.Get<PointLightComponent>(aEntity).SetAttenuationRadius(aRadius); });

		m_pRevertAttenuationRadiusButton->SetIsVisible(!IsAttenuationRadiusDefaultForInspected());
	}

	Ref<ITableRow> PointLightComponentDetailCustomization::OnRequestAttenuationRadiusRow(MAYBE_UNUSED const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			pBox->AddWidget(new Label("Attenuation Radius"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();

			Ref<HorizontalBox> pInnerBox = new HorizontalBox(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetHorizontalSizePolicy(ESizePolicy::Fixed);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));

			pInnerBox->AddWidget(new FloatSlider(0.08f, 163.48f, "%.3f m", ImGuiSliderFlags_Logarithmic))
				->Value(this, &PointLightComponentDetailCustomization::GetAttenuationRadius)
				->OnValueChanged(this, &PointLightComponentDetailCustomization::OnAttenuationRadiusChanged)
				->SetHorizontalSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBox> pBox = new HorizontalBox();
			m_pRevertAttenuationRadiusButton = pBox->AddWidget(AddRevertButtonWidget([this](float aValue) { OnAttenuationRadiusChanged(aValue); }, DEFAULT_ATTENUATION_RADIUS, !IsAttenuationRadiusDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}
}
