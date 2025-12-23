#include "SpotLightComponentDetailCustomization.h"

#include "../LayoutBuilders/EntityDetailLayoutBuilder.h"
#include "../TableRows/EntityDetailRow.h"

namespace Relentless
{
	void SpotLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
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
		attenuationRadiusNode.OnRequestRow(this, &SpotLightComponentDetailCustomization::OnRequestAttenuationRadiusRow);

		DetailNode& innerConeAngleNode = categoryBuilder.AddProperty("Inner Cone Angle");
		innerConeAngleNode.OnRequestRow(this, &SpotLightComponentDetailCustomization::OnRequestInnerConeAngleRow);

		DetailNode& outerConeAngleNode = categoryBuilder.AddProperty("Outer Cone Angle");
		outerConeAngleNode.OnRequestRow(this, &SpotLightComponentDetailCustomization::OnRequestOuterConeAngleRow);

		DetailNode& useTempNode = categoryBuilder.AddProperty("Use Temperature");
		useTempNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestUseTemperatureRow);

		DetailNode& temperatureNode = categoryBuilder.AddProperty("Temperature");
		temperatureNode.OnRequestRow(static_cast<ILightComponentDetailCustomization*>(this), &ILightComponentDetailCustomization::OnRequestTemperatureRow);

		m_pBuilder = &builder;
	}

	float SpotLightComponentDetailCustomization::GetAttenuationRadius() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<SpotLightComponent>(GetPrimaryEntity()).GetAttenuationRadius();
	}

	float SpotLightComponentDetailCustomization::GetInnerConeAngleDegrees() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<SpotLightComponent>(GetPrimaryEntity()).GetInnerConeAngleDegrees();
	}

	float SpotLightComponentDetailCustomization::GetOuterConeAngleDegrees() const noexcept
	{
		return m_pBuilder->GetScene().GetEntityManager().Get<SpotLightComponent>(GetPrimaryEntity()).GetOuterConeAngleDegrees();
	}

	bool SpotLightComponentDetailCustomization::IsAttenuationRadiusDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				auto& slc = entityManager.Get<SpotLightComponent>(aEntity);
				return Math::AreValuesClose(slc.GetAttenuationRadius(), DEFAULT_ATTENUATION_RADIUS);
			});
	}

	bool SpotLightComponentDetailCustomization::IsInnerConeAngleDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				auto& plc = entityManager.Get<SpotLightComponent>(aEntity);
				return Math::AreValuesClose(plc.GetInnerConeAngleDegrees(), DEFAULT_INNER_CONE_ANGLES_DEGREES);
			});
	}

	bool SpotLightComponentDetailCustomization::IsOuterConeAngleDefaultForInspected() const noexcept
	{
		Scene& scene = m_pBuilder->GetScene();
		EntityManager& entityManager = scene.GetEntityManager();

		return std::ranges::all_of(GetInspectedEntities(), [this, &entityManager](entity aEntity)
			{
				auto& plc = entityManager.Get<SpotLightComponent>(aEntity);
				return Math::AreValuesClose(plc.GetOuterConeAngleDegrees(), DEFAULT_OUTER_CONE_ANGLES_DEGREES);
			});
	}

	void SpotLightComponentDetailCustomization::OnAttenuationRadiusChanged(float aRadius) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity) {	entityManager.Get<SpotLightComponent>(aEntity).SetAttenuationRadius(aRadius); });

		m_pRevertAttenuationRadiusButton->SetIsVisible(!IsAttenuationRadiusDefaultForInspected());
	}

	void SpotLightComponentDetailCustomization::OnInnerConeAngleChanged(float aAngleDegrees) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity) 
			{	
				SpotLightComponent& slc = entityManager.Get<SpotLightComponent>(aEntity);
				slc.SetInnerConeAngleDegrees(aAngleDegrees);

				if (slc.GetOuterConeAngleDegrees() < aAngleDegrees)
				{
					slc.SetOuterConeAngleDegrees(aAngleDegrees);
					m_pRevertOuterConeAngleButton->SetIsVisible(!IsOuterConeAngleDefaultForInspected());
				}
			});

		m_pRevertInnerConeAngleButton->SetIsVisible(!IsInnerConeAngleDefaultForInspected());
	}

	void SpotLightComponentDetailCustomization::OnOuterConeAngleChanged(float aAngleDegrees) noexcept
	{
		EntityManager& entityManager = m_pBuilder->GetScene().GetEntityManager();
		std::ranges::for_each(GetInspectedEntities(), [&](entity aEntity) 
			{	
				SpotLightComponent& slc = entityManager.Get<SpotLightComponent>(aEntity);
				slc.SetOuterConeAngleDegrees(aAngleDegrees);

				if (slc.GetInnerConeAngleDegrees() > aAngleDegrees)
				{
					slc.SetInnerConeAngleDegrees(aAngleDegrees);
					m_pRevertInnerConeAngleButton->SetIsVisible(!IsInnerConeAngleDefaultForInspected());
				}
			});

		m_pRevertOuterConeAngleButton->SetIsVisible(!IsOuterConeAngleDefaultForInspected());
	}

	Ref<ITableRow> SpotLightComponentDetailCustomization::OnRequestAttenuationRadiusRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Attenuation Radius"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));

			pInnerBox->AddWidget(new FloatSlider(0.08f, 163.48f, "%.3f m", ImGuiSliderFlags_Logarithmic))
				->Value(this, &SpotLightComponentDetailCustomization::GetAttenuationRadius)
				->OnValueChanged(this, &SpotLightComponentDetailCustomization::OnAttenuationRadiusChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertAttenuationRadiusButton = pBox->AddWidget(AddRevertButtonWidget([this](float aValue) { OnAttenuationRadiusChanged(aValue); }, DEFAULT_ATTENUATION_RADIUS, !IsAttenuationRadiusDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> SpotLightComponentDetailCustomization::OnRequestInnerConeAngleRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Inner Cone Angle"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));

			pInnerBox->AddWidget(new FloatSlider(0.0f, 80.0f, "%.3f"))
				->Value(this, &SpotLightComponentDetailCustomization::GetInnerConeAngleDegrees)
				->OnValueChanged(this, &SpotLightComponentDetailCustomization::OnInnerConeAngleChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertInnerConeAngleButton = pBox->AddWidget(AddRevertButtonWidget([this](float aValue) { OnInnerConeAngleChanged(aValue); }, DEFAULT_INNER_CONE_ANGLES_DEGREES, !IsInnerConeAngleDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

	Ref<ITableRow> SpotLightComponentDetailCustomization::OnRequestOuterConeAngleRow(const ItemInfo& aItemInfo) noexcept
	{
		Ref<EntityDetailRow> pRow = new EntityDetailRow();

		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			pBox->AddWidget(new Label("Outer Cone Angle"));
			pRow->SetColumnWidget(0, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();

			Ref<HorizontalBoxEx> pInnerBox = new HorizontalBoxEx(Vector2(140.0f, 32.0f), true);
			pInnerBox->SetSpacing(0.0f);
			pInnerBox->SetSizePolicy(ESizePolicy::Fixed);
			pInnerBox->SetMargin(FloatRect(0.0f, 3.0f, 0.0f, 3.0f));

			pInnerBox->AddWidget(new FloatSlider(0.0f, 80.0f, "%.3f"))
				->Value(this, &SpotLightComponentDetailCustomization::GetOuterConeAngleDegrees)
				->OnValueChanged(this, &SpotLightComponentDetailCustomization::OnOuterConeAngleChanged)
				->SetSizePolicy(ESizePolicy::Stretch);

			pBox->AddWidget(pInnerBox);
			pRow->SetColumnWidget(1, pBox);
		}
		{
			Ref<HorizontalBoxEx> pBox = new HorizontalBoxEx();
			m_pRevertOuterConeAngleButton = pBox->AddWidget(AddRevertButtonWidget([this](float aValue) { OnOuterConeAngleChanged(aValue); }, DEFAULT_OUTER_CONE_ANGLES_DEGREES, !IsOuterConeAngleDefaultForInspected()));
			pRow->SetColumnWidget(2, pBox);
		}

		return pRow;
	}

}
