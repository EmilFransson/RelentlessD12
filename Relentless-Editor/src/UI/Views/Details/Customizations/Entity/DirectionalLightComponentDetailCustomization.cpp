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
			[](entity, DirectionalLightComponent& aDLC, const float& aIntensity) { aDLC.SetIntensityLux(aIntensity); },
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
			[](entity, DirectionalLightComponent& aDLC, const Color& aColor) { aDLC.SetColor(aColor); },
			Colors::White
		);

		categoryBuilder.AddProperty<Color>("Light Color", pColorHandle)
			.NameSlot().Label("Light Color")
			.ValueSlot().ColorPicker();

		Ref<EntityPropertyHandle<bool, DirectionalLightComponent>> pUseTemperatureHandle = RLS_NEW EntityPropertyHandle<bool, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.IsUsingTemperature(); },
			[](entity, DirectionalLightComponent& aDLC, const bool& aState) { aDLC.SetUseTemperature(aState); },
			false
		);

		categoryBuilder.AddProperty<bool>("Use Temperature", pUseTemperatureHandle)
			.NameSlot().Label("Use Temperature")
			.ValueSlot().CheckBox();

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pTemperatureHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetTemperature(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aTemperature) { aDLC.SetTemperature(aTemperature); },
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

		categoryBuilder.AddProperty<bool>("Lighting Channels", nullptr)
			.NameSlot().Label("Lighting Channels")
			.ValueSlot().Widget([&context]()
				{
					auto&& AllHaveChannelsSet = [&context](ELightChannel aLightChannel) -> bool
						{
							return std::ranges::all_of(context.Entities, [&context, aLightChannel](entity aEntity)
								{
									const DirectionalLightComponent& directionalLightComponent = context.EntityManager->Get<DirectionalLightComponent>(aEntity);
									return directionalLightComponent.HasAnyChannel(aLightChannel);
								});
						};

					Ref<HorizontalBox> pBox = RLS_NEW HorizontalBox();
					pBox->SetPadding({ 0.0f, 2.0f, 0.0f, 2.0f });
					pBox->SetSpacing(5.0f);

					for (uint32 i = 0; i <= 5; ++i)
					{
						const ELightChannel lightChannel = static_cast<ELightChannel>(1u << i);

						Button* pButton = pBox->AddWidget(RLS_NEW Button(std::format("{}", i)));
						pButton->SetVerticalAlignmentPolicy(EVerticalAlignmentPolicy::Center);
						pButton->SetHorizontalSizePolicy(ESizePolicy::Fixed);
						pButton->SetVerticalSizePolicy(ESizePolicy::Fixed);
						pButton->SetSize({ 20.0f, 20.0f });
						pButton->OnClicked([&context, lightChannel, pButton, AllHaveChannelsSet]()
							{
								const bool setChannel = !AllHaveChannelsSet(lightChannel);

								std::ranges::for_each(context.Entities, [&context, lightChannel, setChannel](entity aEntity)
									{
										DirectionalLightComponent& directionalLightComponent = context.EntityManager->Get<DirectionalLightComponent>(aEntity);
										directionalLightComponent.SetChannelEnabled(lightChannel, setChannel);
									});
								
								pButton->SetBackgroundColor(setChannel ? Colors::Blue : Colors::Black);
								pButton->SetHoverColor(setChannel ? Colors::Blue : Colors::Black);

								if (setChannel)
									pButton->SetTextColor(Colors::White);
							});

						pButton->OnMouseEnter([](Button* aButton) { aButton->SetTextColor(Colors::White); });
						pButton->OnMouseExit([lightChannel, AllHaveChannelsSet](Button* aButton)
							{ 
								if (AllHaveChannelsSet(lightChannel))
									return;

								aButton->SetTextColor(Colors::Gray); 
							});

						const bool allSet = AllHaveChannelsSet(lightChannel);
						pButton->SetBackgroundColor(allSet ? Colors::Blue : Colors::Black);
						pButton->SetHoverColor(allSet ? Colors::Blue : Colors::Black);
						pButton->SetBorderColor(Colors::Normalize(50.0f, 50.0f, 50.0f, 255.0f));
						pButton->SetTextColor(allSet ? Colors::White : Colors::Gray);
					}

					return pBox;
				});

		IDetailGroupBuilder shadowsGroupBuilder = categoryBuilder.EditGroup("Shadows");

		Ref<EntityPropertyHandle<bool, DirectionalLightComponent>> pCastShadowsHandle = RLS_NEW EntityPropertyHandle<bool, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.IsCastingShadows(); },
			[](entity, DirectionalLightComponent& aDLC, const bool& aCastShadows) { aDLC.SetCastShadows(aCastShadows); },
			true
		);

		shadowsGroupBuilder.AddProperty<bool>("Cast Shadows", pCastShadowsHandle)
			.NameSlot().Label("Cast Shadows")
			.ValueSlot().CheckBox();

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pShadowAmountHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetShadowAmount(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aShadowAmount) { aDLC.SetShadowAmount(aShadowAmount); },
			1.0f
		);

		shadowsGroupBuilder.AddProperty<float>("Shadow Amount", pShadowAmountHandle)
			.NameSlot().Label("Shadow Amount")
			.ValueSlot().Slider().Range(0.0f, 1.0f);

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pShadowResolutionScaleHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetShadowResolutionScale(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aShadowResolutionScale) { aDLC.SetShadowResolutionScale(aShadowResolutionScale); },
			1.0f
		);

		shadowsGroupBuilder.AddProperty<float>("Shadow Resolution Scale", pShadowResolutionScaleHandle)
			.NameSlot().Label("Shadow Resolution Scale")
			.ValueSlot().Slider().Range(0.125f, 8.0f);

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pShadowBiasHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetShadowBias(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aShadowBias) { aDLC.SetShadowBias(aShadowBias); },
			0.0001f
		);

		shadowsGroupBuilder.AddProperty<float>("Shadow Bias", pShadowBiasHandle)
			.NameSlot().Label("Shadow Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 0.1f);

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pShadowSlopeBiasHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetShadowSlopeBias(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aShadowSlopeBias) { aDLC.SetShadowSlopeBias(aShadowSlopeBias); },
			0.0005f
		);

		shadowsGroupBuilder.AddProperty<float>("Shadow Slope Bias", pShadowSlopeBiasHandle)
			.NameSlot().Label("Shadow Slope Bias")
			.ValueSlot().Slider().Logarithmic(true).Range(0.0f, 0.1f);

		Ref<EntityPropertyHandle<uint32, DirectionalLightComponent>> pNumCascadesHandle = RLS_NEW EntityPropertyHandle<uint32, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetNumCascades(); },
			[](entity, DirectionalLightComponent& aDLC, const uint32& aNumCascades) { aDLC.SetNumCascades(aNumCascades); },
			4u
		);

		shadowsGroupBuilder.AddProperty<uint32>("Number of Cascades", pNumCascadesHandle)
			.NameSlot().Label("Number of Cascades")
			.ValueSlot().Slider().Range(1u, 4u);

		Ref<EntityPropertyHandle<float, DirectionalLightComponent>> pCascadeDistributionHandle = RLS_NEW EntityPropertyHandle<float, DirectionalLightComponent>(
			*context.EntityManager,
			context.Entities,
			[](const DirectionalLightComponent& aDLC) { return aDLC.GetCascadeDistribution(); },
			[](entity, DirectionalLightComponent& aDLC, const float& aCascadeDistribution) { aDLC.SetCascadeDistribution(aCascadeDistribution); },
			0.85f
		);

		shadowsGroupBuilder.AddProperty<float>("Cascade Distribution", pCascadeDistributionHandle)
			.NameSlot().Label("Cascade Distribution")
			.ValueSlot().Slider().Range(0.0f, 1.0f);
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