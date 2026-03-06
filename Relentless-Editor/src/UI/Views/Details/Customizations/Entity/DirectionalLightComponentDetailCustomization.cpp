#include "DirectionalLightComponentDetailCustomization.h"

#include <Relentless.h>

#include "UI/Views/Details/IDetailsView.h"
#include "UI/Views/Details/LayoutBuilders/IDetailLayoutBuilder.h"
#include "UI/Views/Details/LayoutBuilders/IDetailCategoryBuilder.h"
#include "UI/Views/Details/Context/EntityDetailsContext.h"

#include "Property/EntityPropertyHandle.h"

namespace Relentless
{
	void DirectionalLightComponentDetailCustomization::CustomizeDetails(IDetailLayoutBuilder& aDetailLayoutBuilder) noexcept
	{
		EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		const bool multiSelection = context.Entities.size() > 1u;

		Ref<EntityPropertyHandle<int, DirectionalLightComponent>> pTypeHandle = RLS_NEW EntityPropertyHandle<int, DirectionalLightComponent>(
			*context.EntityManager, 
			context.Entities,
			[](MAYBE_UNUSED const DirectionalLightComponent& aDLC) { return 0; },
			[&context, pDetailsView = aDetailLayoutBuilder.GetDetailsView()](entity aEntity, DirectionalLightComponent&, const int& aSelection)
			{  
				if (aSelection == 1)
				{
					Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity]()
						{
							const DirectionalLightComponent& directionalLightComponent = context.EntityManager->Get<DirectionalLightComponent>(aEntity);

							PointLightComponent& pointLightComponent = context.EntityManager->Add<PointLightComponent>(aEntity);
							pointLightComponent.SetColor(directionalLightComponent.GetColor());
							pointLightComponent.SetIntensityCandela(Math::Min(160.0f, directionalLightComponent.GetIntensity()));
							pointLightComponent.SetTemperature(directionalLightComponent.GetTemperature());
							pointLightComponent.SetUseTemperature(directionalLightComponent.IsUsingTemperature());

							context.EntityManager->Remove<DirectionalLightComponent>(aEntity);
							pDetailsView->RequestRefresh();
						});
				}
				else //2
				{
					Application::Get().SubmitToMainThread([&context, pDetailsView, aEntity]()
						{
							const DirectionalLightComponent& directionalLightComponent = context.EntityManager->Get<DirectionalLightComponent>(aEntity);

							SpotLightComponent& spotLightComponent = context.EntityManager->Add<SpotLightComponent>(aEntity);
							spotLightComponent.SetColor(directionalLightComponent.GetColor());
							spotLightComponent.SetIntensityCandela(Math::Min(160.0f, directionalLightComponent.GetIntensity()));
							spotLightComponent.SetTemperature(directionalLightComponent.GetTemperature());
							spotLightComponent.SetUseTemperature(directionalLightComponent.IsUsingTemperature());

							context.EntityManager->Remove<DirectionalLightComponent>(aEntity);
							pDetailsView->RequestRefresh();
						});
				}
			}
			);

		IDetailCategoryBuilder& categoryBuilder = aDetailLayoutBuilder.EditCategory(ICON_FA_LIGHTBULB "  Light");
		categoryBuilder.AddProperty<int>("Type", pTypeHandle)
			.NameSlot().Label("Type")
			.ValueSlot().ComboBox().Options({ "Directional", "Point", "Spot" }).Selected(0);

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pIntensityHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetIntensity(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aIntensity) { return aDLC.SetIntensityLux(aIntensity); },
			100'000.0f
		);

		auto intensityBuilder = categoryBuilder.AddProperty<float>("Intensity", pIntensityHandle);
		intensityBuilder.NameSlot().Label("Intensity");
		if (multiSelection)
			intensityBuilder.ValueSlot().NumericEntryBox().Range(0.0f, 120'000.0f).Unit(" lux");
		else 
			intensityBuilder.ValueSlot().Slider().Range(0.0f, 120'000.0f).Unit(" lux");

		Ref<EntityPropertyHandle<Color, DirectionalLightComponent>> pColorHandle = RLS_NEW EntityPropertyHandle<Color, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetColor(); },
			[](entity, DirectionalLightComponent& aDLC, const Color& aColor) { return aDLC.SetColor(aColor); },
			Colors::White
		);

		categoryBuilder.AddProperty<Color>("Light Color", pColorHandle)
			.NameSlot().Label("Light Color")
			.ValueSlot().ColorPicker();

		Ref<EntityPropertyHandle<bool, DirectionalLightComponent>> pUseTemperatureHandle = RLS_NEW EntityPropertyHandle<bool, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.IsUsingTemperature(); },
			[](entity, DirectionalLightComponent& aDLC, const bool& aState) { return aDLC.SetUseTemperature(aState); },
			false
		);

		categoryBuilder.AddProperty<bool>("Use Temperature", pUseTemperatureHandle)
			.NameSlot().Label("Use Temperature")
			.ValueSlot().CheckBox();

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pTemperatureHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetTemperature(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aTemperature) { return aDLC.SetTemperature(aTemperature); },
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

	bool DirectionalLightComponentDetailCustomization::ShouldCustomize(IDetailLayoutBuilder& aDetailLayoutBuilder) const noexcept
	{
		const EntityDetailsContext& context = aDetailLayoutBuilder.GetDetailsView()->GetContext<EntityDetailsContext>();
		if (context.Entities.empty())
			return false;

		if (!std::ranges::all_of(context.Entities, [&context](entity aEntity) { return context.EntityManager->Has<DirectionalLightComponent>(aEntity); }))
			return false;

		return true;
	}

}