#include "PointLightComponentDetailCustomization.h"

#include <Relentless.h>

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	void PointLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		const bool multiSelection = context.Entities.size() > 1u;

		Ref<EntityPropertyHandle<int, PointLightComponent>> pTypeHandle = RLS_NEW EntityPropertyHandle<int, PointLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](MAYBE_UNUSED const PointLightComponent& aDLC) { return 1; },
			[&context, pDetailsView = aDetailLayoutBuilder.GetDetailsView()](entity aEntity, PointLightComponent&, const int& aSelection)
			{
				if (aSelection == 0)
				{
					Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity]()
						{
							const PointLightComponent& pointLightComponent = context.EntityManager->Get<PointLightComponent>(aEntity);
							
							DirectionalLightComponent& directionalLightComponent = context.EntityManager->Add<DirectionalLightComponent>(aEntity);
							directionalLightComponent.SetColor(pointLightComponent.GetColor());
							directionalLightComponent.SetIntensityLux(pointLightComponent.GetIntensity());
							directionalLightComponent.SetTemperature(pointLightComponent.GetTemperature());
							directionalLightComponent.SetUseTemperature(pointLightComponent.IsUsingTemperature());

							context.EntityManager->Remove<PointLightComponent>(aEntity);
							pDetailsView->RequestRefresh();
						});
				}
				else //2
				{
					Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity]()
						{
							const PointLightComponent& pointLightComponent = context.EntityManager->Get<PointLightComponent>(aEntity);

							SpotLightComponent& spotLightComponent = context.EntityManager->Add<SpotLightComponent>(aEntity);
							spotLightComponent.SetColor(pointLightComponent.GetColor());
							spotLightComponent.SetIntensityCandela(pointLightComponent.GetIntensity());
							spotLightComponent.SetTemperature(pointLightComponent.GetTemperature());
							spotLightComponent.SetUseTemperature(pointLightComponent.IsUsingTemperature());

							context.EntityManager->Remove<PointLightComponent>(aEntity);
							pDetailsView->RequestRefresh();
						});
				}
			}
		);

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_LIGHTBULB "  Light");
		
		categoryBuilder.AddProperty<int>("Type", pTypeHandle)
			.NameSlot().Label("Type")
			.ValueSlot().ComboBox().Options({ "Directional", "Point", "Spot" }).Selected(1);

		Ref<EntityPropertyHandle<float, PointLightComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, PointLightComponent>(
			*context.EntityManager,
			context.Entities,
			[&context](const PointLightComponent& aPLC) 
			{ 
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					return aPLC.GetIntensity();
				else
					return Math::Photometry::CandelaToLumen_Point(aPLC.GetIntensity());
			},
			[&context](entity, PointLightComponent& aPLC, const float& aIntensity)
			{
				if (context.LightIntensityType == ELightIntensityType::Candelas)
					aPLC.SetIntensityCandela(aIntensity);
				else
					aPLC.SetIntensityLumen(aIntensity);
			},
			context.LightIntensityType == ELightIntensityType::Candelas ? 8.0f : Math::Photometry::CandelaToLumen_Point(8.0f)
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
			.ValueSlot().ComboBox().Options({"Candelas", "Lumens"}).Selected(static_cast<int>(context.LightIntensityType));

		Ref<EntityPropertyHandle<Color, PointLightComponent>> pColorHandle = RLS_NEW EntityPropertyHandle<Color, PointLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const PointLightComponent& aPLC) { return aPLC.GetColor(); },
			[](entity, PointLightComponent& aPLC, const Color& aColor) { return aPLC.SetColor(aColor); },
			Colors::White
		);

		categoryBuilder.AddProperty<Color>("Light Color", pColorHandle)
			.NameSlot().Label("Light Color")
			.ValueSlot().ColorPicker();

		Ref<EntityPropertyHandle<float, PointLightComponent>> pAttenuationRadiusHandle = RLS_NEW EntityPropertyHandle<float, PointLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const PointLightComponent& aPLC) { return aPLC.GetAttenuationRadius(); },
			[](entity, PointLightComponent& aPLC, const float& aAttenuationRadius) { return aPLC.SetAttenuationRadius(aAttenuationRadius); },
			10.0f
		);

		auto attenuationBuilder = categoryBuilder.AddProperty<float>("Attenuation Radius", pAttenuationRadiusHandle);
		attenuationBuilder.NameSlot().Label("Attenuation Radius");
		if (multiSelection)
			attenuationBuilder.ValueSlot().NumericEntryBox().Range(0.08f, 163.48f).Unit(" m");
		else
			attenuationBuilder.ValueSlot().Slider().Range(0.08f, 163.48f).Unit(" m").Logarithmic(true);

		Ref<EntityPropertyHandle<bool, PointLightComponent>> pUseTemperatureHandle = RLS_NEW EntityPropertyHandle<bool, PointLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const PointLightComponent& aPLC) { return aPLC.IsUsingTemperature(); },
			[](entity, PointLightComponent& aPLC, const bool& aState) { return aPLC.SetUseTemperature(aState); },
			false
		);

		categoryBuilder.AddProperty<bool>("Use Temperature", pUseTemperatureHandle)
			.NameSlot().Label("Use Temperature")
			.ValueSlot().CheckBox();

		Ref<EntityPropertyHandle<float, PointLightComponent>> pTemperatureHandle = RLS_NEW EntityPropertyHandle<float, PointLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const PointLightComponent& aPLC) { return aPLC.GetTemperature(); },
			[](entity, PointLightComponent& aPLC, const float& aTemperature) { return aPLC.SetTemperature(aTemperature); },
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

	bool PointLightComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		if (!std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<PointLightComponent>(aEntity); }))
			return false;

		return true;
	}

}