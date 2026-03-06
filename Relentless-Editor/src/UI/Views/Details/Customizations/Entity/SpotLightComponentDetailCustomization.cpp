#include "SpotLightComponentDetailCustomization.h"

#include <Relentless.h>

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	void SpotLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		const bool multiSelection = context.Entities.size() > 1u;

		Ref<EntityPropertyHandle<int, SpotLightComponent>> pTypeHandle = RLS_NEW EntityPropertyHandle<int, SpotLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](MAYBE_UNUSED const SpotLightComponent& aSLC) { return 2; },
			[&context, pDetailsView = aDetailLayoutBuilder.GetDetailsView()](entity aEntity, SpotLightComponent&, const int& aSelection)
			{
				if (aSelection == 0)
				{
					Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity]()
						{
							const SpotLightComponent& spotLightComponent = context.EntityManager->Get<SpotLightComponent>(aEntity);

							DirectionalLightComponent& directionalLightComponent = context.EntityManager->Add<DirectionalLightComponent>(aEntity);
							directionalLightComponent.SetColor(spotLightComponent.GetColor());
							directionalLightComponent.SetIntensityLux(spotLightComponent.GetIntensity());
							directionalLightComponent.SetTemperature(spotLightComponent.GetTemperature());
							directionalLightComponent.SetUseTemperature(spotLightComponent.IsUsingTemperature());

							context.EntityManager->Remove<SpotLightComponent>(aEntity);
							pDetailsView->RequestRefresh();
						});
				}
				else //1
				{
					Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity]()
						{
							const SpotLightComponent& spotLightComponent = context.EntityManager->Get<SpotLightComponent>(aEntity);

							PointLightComponent& pointLightComponent = context.EntityManager->Add<PointLightComponent>(aEntity);
							pointLightComponent.SetColor(spotLightComponent.GetColor());
							pointLightComponent.SetIntensityCandela(spotLightComponent.GetIntensity());
							pointLightComponent.SetTemperature(spotLightComponent.GetTemperature());
							pointLightComponent.SetUseTemperature(spotLightComponent.IsUsingTemperature());

							context.EntityManager->Remove<SpotLightComponent>(aEntity);
							pDetailsView->RequestRefresh();
						});
				}
			}
		);

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_LIGHTBULB "  Light");

		categoryBuilder.AddProperty<int>("Type", pTypeHandle)
			.NameSlot().Label("Type")
			.ValueSlot().ComboBox().Options({ "Directional", "Point", "Spot" }).Selected(2);

		Ref<EntityPropertyHandle<float, SpotLightComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, SpotLightComponent>(
			*context.EntityManager,
			context.Entities,
			[&context](const SpotLightComponent& aSLC)
			{
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					return aSLC.GetIntensity();
				else
					return Math::Photometry::CandelaToLumen_Spot(aSLC.GetIntensity(), aSLC.GetOuterConeAngleRadians());
			},
			[&context](entity, SpotLightComponent& aSLC, const float& aIntensity)
			{
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					aSLC.SetIntensityCandela(aIntensity);
				else
					aSLC.SetIntensityLumen(aIntensity);
			},
			context.LightIntensityType == ELightIntensityType::Candelas ? 8.0f : Math::Photometry::CandelaToLumen_Spot(8.0f, Math::DegToRad(44.0f)) 
		);

		const char* unit = context.LightIntensityType == ELightIntensityType::Candelas ? " cd" : " lm";
		const Vector2 range = context.LightIntensityType == ELightIntensityType::Candelas ? Vector2(0.0f, 160.0f) : Vector2(0.0f, 2'010.619263f);

		auto intensityBuilder = categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle);
		intensityBuilder.NameSlot().Label("Intensity");
		if (multiSelection)
			intensityBuilder.ValueSlot().NumericEntryBox().Range(range.x, range.y).Unit(unit);
		else
			intensityBuilder.ValueSlot().Slider().Range(range.x, range.y).Unit(unit);

		Ref<PropertyHandle<int>> pIntensityUnitsHandle = RLS_NEW PropertyHandle<int>(
			[&context]() { return static_cast<int>(context.LightIntensityType); },
			[&context, pDetailsView = aDetailLayoutBuilder.GetDetailsView()](const int& aValue)
			{
				context.LightIntensityType = static_cast<ELightIntensityType>(aValue);
				pDetailsView->RequestRefresh();
			},
			0
		);

		categoryBuilder.AddProperty<int>("Intensity Units", pIntensityUnitsHandle)
			.NameSlot().Label("Intensity Units")
			.ValueSlot().ComboBox().Options({ "Candelas", "Lumens" }).Selected(static_cast<int>(context.LightIntensityType));

		Ref<EntityPropertyHandle<Color, SpotLightComponent>> pColorHandle = RLS_NEW EntityPropertyHandle<Color, SpotLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SpotLightComponent& aSLC) { return aSLC.GetColor(); },
			[](entity, SpotLightComponent& aSLC, const Color& aColor) { return aSLC.SetColor(aColor); },
			Colors::White
		);

		categoryBuilder.AddProperty<Color>("Light Color", pColorHandle)
			.NameSlot().Label("Light Color")
			.ValueSlot().ColorPicker();

		Ref<EntityPropertyHandle<float, SpotLightComponent>> pAttenuationRadiusHandle = RLS_NEW EntityPropertyHandle<float, SpotLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SpotLightComponent& aSLC) { return aSLC.GetAttenuationRadius(); },
			[](entity, SpotLightComponent& aSLC, const float& aAttenuationRadius) { return aSLC.SetAttenuationRadius(aAttenuationRadius); },
			10.0f
		);

		auto attenuationBuilder = categoryBuilder.AddProperty<float>("Attenuation Radius", pAttenuationRadiusHandle);
		attenuationBuilder.NameSlot().Label("Attenuation Radius");
		if (multiSelection)
			attenuationBuilder.ValueSlot().NumericEntryBox().Range(0.08f, 163.48f).Unit(" m");
		else
			attenuationBuilder.ValueSlot().Slider().Range(0.08f, 163.48f).Unit(" m").Logarithmic(true);

		//Inner Cone angle:
		{
			Ref<EntityPropertyHandle<float, SpotLightComponent>> pInnerConeAngleHandle = RLS_NEW EntityPropertyHandle<float, SpotLightComponent>(
				*context.EntityManager,
				context.Entities,
				[](const SpotLightComponent& aSLC) { return aSLC.GetInnerConeAngleDegrees(); },
				[](entity, SpotLightComponent& aSLC, const float& aInnerConeAngleDegrees) { return aSLC.SetInnerConeAngleDegrees(aInnerConeAngleDegrees); },
				0.0f
			);

			auto innerConeAngleBuilder = categoryBuilder.AddProperty<float>("Inner Cone Angle", pInnerConeAngleHandle);
			innerConeAngleBuilder.NameSlot().Label("Inner Cone Angle");
			if (multiSelection)
				innerConeAngleBuilder.ValueSlot().NumericEntryBox().Range(0.0f, 80.0f).Unit("\xC2\xB0");
			else
				innerConeAngleBuilder.ValueSlot().Slider().Range(0.0f, 80.0f).Unit("\xC2\xB0");
		}

		//Outer Cone Angle:
		{
			Ref<EntityPropertyHandle<float, SpotLightComponent>> pOuterConeAngleHandle = RLS_NEW EntityPropertyHandle<float, SpotLightComponent>(
				*context.EntityManager,
				context.Entities,
				[](const SpotLightComponent& aSLC) { return aSLC.GetOuterConeAngleDegrees(); },
				[](entity, SpotLightComponent& aSLC, const float& aOuterConeAngleDegrees) { return aSLC.SetOuterConeAngleDegrees(aOuterConeAngleDegrees); },
				44.0f
			);

			auto builder = categoryBuilder.AddProperty<float>("Outer Cone Angle", pOuterConeAngleHandle);
			builder.NameSlot().Label("Outer Cone Angle");
			if (multiSelection)
				builder.ValueSlot().NumericEntryBox().Range(0.0f, 80.0f).Unit("\xC2\xB0");
			else
				builder.ValueSlot().Slider().Range(0.0f, 80.0f).Unit("\xC2\xB0");
		}

		Ref<EntityPropertyHandle<bool, SpotLightComponent>> pUseTemperatureHandle = RLS_NEW EntityPropertyHandle<bool, SpotLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SpotLightComponent& aSLC) { return aSLC.IsUsingTemperature(); },
			[](entity, SpotLightComponent& aSLC, const bool& aState) { return aSLC.SetUseTemperature(aState); },
			false
		);

		categoryBuilder.AddProperty<bool>("Use Temperature", pUseTemperatureHandle)
			.NameSlot().Label("Use Temperature")
			.ValueSlot().CheckBox();

		Ref<EntityPropertyHandle<float, SpotLightComponent>> pTemperatureHandle = RLS_NEW EntityPropertyHandle<float, SpotLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const SpotLightComponent& aSLC) { return aSLC.GetTemperature(); },
			[](entity, SpotLightComponent& aSLC, const float& aTemperature) { return aSLC.SetTemperature(aTemperature); },
			6'500.0f
		);

		auto temperatureBuilder = categoryBuilder.AddProperty<float>("Temperature", pTemperatureHandle);
		temperatureBuilder.NameSlot().Label("Temperature");

		if (multiSelection)
		{
			temperatureBuilder.ValueSlot().Widget([useTemperatureHandle = pUseTemperatureHandle.Get(), pTemperatureHandle]()
				{
					Ref<NumericEntryBox<float>> pNumericEntryBox = RLS_NEW NumericEntryBox<float>();
					pNumericEntryBox->SetMinValue(1'700.0f);
					pNumericEntryBox->SetMaxValue(12'000.0f);
					pNumericEntryBox->SetSuffix(" K");
					pNumericEntryBox->Bind(pTemperatureHandle);
					pNumericEntryBox->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
					pNumericEntryBox->SetHorizontalSizePolicy(ESizePolicy::Stretch);
					pNumericEntryBox->SetSteppingEnabled(false);

					bool initialEnabledState = false;
					useTemperatureHandle->GetValue(initialEnabledState);
					pNumericEntryBox->SetIsEnabled(initialEnabledState);

					useTemperatureHandle->OnValueChanged.Connect([slider = pNumericEntryBox.Get()](const bool& aState)
						{
							slider->SetIsEnabled(aState);
						});

					return pNumericEntryBox;
				});
		}
		else
		{
			temperatureBuilder.ValueSlot().Widget([useTemperatureHandle = pUseTemperatureHandle.Get(), pTemperatureHandle]()
				{
					Ref<Slider<float>> pSlider = RLS_NEW Slider<float>();
					pSlider->SetMinValue(1'700.0f);
					pSlider->SetMaxValue(12'000.0f);
					pSlider->SetSuffix(" K");
					pSlider->Bind(pTemperatureHandle);
					pSlider->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
					pSlider->SetHorizontalSizePolicy(ESizePolicy::Stretch);

					bool initialEnabledState = false;
					useTemperatureHandle->GetValue(initialEnabledState);
					pSlider->SetIsEnabled(initialEnabledState);

					useTemperatureHandle->OnValueChanged.Connect([slider = pSlider.Get()](const bool& aState)
						{
							slider->SetIsEnabled(aState);
						});

					return pSlider;
				});
		}
	}

	bool SpotLightComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		if (!std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<SpotLightComponent>(aEntity); }))
			return false;

		return true;
	}

}